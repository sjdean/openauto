cmake_minimum_required(VERSION 3.16)

# ─── Version (single source of truth) ────────────────────────────────────────
# Numeric triplet only — used for VERSION/SOVERSION.
# Display string (with date/git) is separate and never used for file naming.
set(LIBRARY_BUILD_DATE          "20260328")
set(LIBRARY_BUILD_MAJOR_RELEASE 4)
set(LIBRARY_BUILD_MINOR_RELEASE 1)
set(LIBRARY_BUILD_INCREMENTAL   4)

set(LIBRARY_BUILD_VERSION_STRING
        "${LIBRARY_BUILD_MAJOR_RELEASE}.${LIBRARY_BUILD_MINOR_RELEASE}.${LIBRARY_BUILD_INCREMENTAL}+${LIBRARY_BUILD_DATE}")

project(openauto
        VERSION "${LIBRARY_BUILD_MAJOR_RELEASE}.${LIBRARY_BUILD_MINOR_RELEASE}.${LIBRARY_BUILD_INCREMENTAL}"
        DESCRIPTION "OpenAuto - Android Auto head unit"
        LANGUAGES CXX
)

# ─── Language standard ────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ─── Qt auto-tools ───────────────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ─── Module path ─────────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

# ─── GNU install dirs ─────────────────────────────────────────────────────────
include(GNUInstallDirs)

# ─── Default build type ───────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Defaulting to Release build")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# ─── Platform detection ───────────────────────────────────────────────────────
# Use CMAKE_SYSTEM_NAME directly — avoids relying on the LINUX variable which
# requires CMake 3.25+. We target 3.16 minimum.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OPENAUTO_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OPENAUTO_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OPENAUTO_WINDOWS TRUE)
endif()

message(STATUS "Build type      : ${CMAKE_BUILD_TYPE}")
message(STATUS "System          : ${CMAKE_SYSTEM_NAME} / ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling : YES (target: ${CMAKE_SYSTEM_PROCESSOR})")
endif()

# ─── Git metadata (display only — never used in file/library names) ───────────
# Gracefully degrades when .git is absent (Yocto work dir, CI tarballs, etc.)
find_package(Git QUIET)
if(Git_FOUND)
    execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE   _git_short_hash
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE   _git_branch
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
if(NOT _git_short_hash OR _git_short_hash STREQUAL "")
    set(_git_short_hash "unknown")
endif()
if(NOT _git_branch OR _git_branch STREQUAL "")
    set(_git_branch "unknown")
endif()
if(NOT _git_short_hash STREQUAL "unknown")
    set(LIBRARY_BUILD_VERSION_STRING
            "${LIBRARY_BUILD_MAJOR_RELEASE}.${LIBRARY_BUILD_MINOR_RELEASE}.${LIBRARY_BUILD_INCREMENTAL}+${LIBRARY_BUILD_DATE}.g${_git_short_hash}")
endif()
message(STATUS "Version         : ${LIBRARY_BUILD_VERSION_STRING} (branch: ${_git_branch})")

# ─── ccache ───────────────────────────────────────────────────────────────────
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "ccache          : ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# ─── Qt 6 ─────────────────────────────────────────────────────────────────────
# QT6_INSTALL_DIR is for local desktop builds only.
# In Yocto, Qt is in the sysroot automatically — this option is ignored.
if(NOT "${QT6_INSTALL_DIR}" STREQUAL "")
    if(EXISTS "${QT6_INSTALL_DIR}")
        list(APPEND CMAKE_PREFIX_PATH "${QT6_INSTALL_DIR}")
        message(STATUS "Qt6 prefix      : ${QT6_INSTALL_DIR}")
    else()
        message(WARNING "QT6_INSTALL_DIR points to non-existent path: ${QT6_INSTALL_DIR}")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS
        Core Gui Qml Quick QuickEffects Bluetooth DBus
        Network Multimedia MultimediaWidgets Widgets
)

# ─── aap_protobuf and aasdk ───────────────────────────────────────────────────
# Both ship proper CMake config packages (Config.cmake.in + find_dependency).
# CONFIG mode is required here. The legacy Find*.cmake modules in cmake_modules/
# (Findaap_protobuf.cmake, Findaasdk.cmake) are superseded — do not use them.
# They search for hardcoded filenames (GalConstants.pb.h, Version.hpp) that are
# not guaranteed to exist and create raw path variables rather than imported
# targets, which breaks Yocto cross-sysroot builds.
find_package(aap_protobuf REQUIRED CONFIG)   # → AAPProto::aap_protobuf
find_package(aasdk        REQUIRED CONFIG)   # → AASDK::aasdk

# ─── Abseil ───────────────────────────────────────────────────────────────────
# Protobuf 5.x / 30.x inlines ABSL_CHECK / ABSL_LOG calls in its public headers.
# The aap_protobuf cmake config does not list abseil in INTERFACE_LINK_LIBRARIES,
# so any TU that includes protobuf headers fails to link without this.
find_package(absl REQUIRED CONFIG)

# ─── Boost ────────────────────────────────────────────────────────────────────
find_package(Boost REQUIRED COMPONENTS system log)

# ─── OpenSSL ──────────────────────────────────────────────────────────────────
# On macOS, Apple ships LibreSSL at the standard library paths. CMake 4.x's
# FindOpenSSL may find it but fail to create IMPORTED targets (OpenSSL::SSL /
# OpenSSL::Crypto) because LibreSSL is not a supported OpenSSL provider.
# Steer CMake toward Homebrew's openssl@3 before the find_package call.
# OPENSSL_ROOT_DIR can still be overridden from the command line / CLion settings.
if(APPLE AND NOT DEFINED OPENSSL_ROOT_DIR)
    foreach(_ossl_candidate
            "/usr/local/opt/openssl@3"   # Homebrew Intel
            "/opt/homebrew/opt/openssl@3" # Homebrew Apple Silicon
            "/usr/local/opt/openssl"
            "/opt/homebrew/opt/openssl")
        if(EXISTS "${_ossl_candidate}")
            set(OPENSSL_ROOT_DIR "${_ossl_candidate}" CACHE PATH "Homebrew OpenSSL root")
            message(STATUS "OpenSSL root    : ${OPENSSL_ROOT_DIR} (auto-detected)")
            break()
        endif()
    endforeach()
endif()
find_package(OpenSSL REQUIRED)

# ─── libusb — three-tier: config → pkg-config → manual ───────────────────────
# Yocto does not install CMake config files for libusb1, and pkg-config is a
# native tool unavailable in the cross sysroot. The manual fallback using
# find_library/find_path always works because Yocto stages headers and libs
# into recipe-sysroot at known FHS paths, which CMake searches via sysroot.
find_package(libusb-1.0 QUIET CONFIG)
if(libusb-1.0_FOUND OR TARGET libusb-1.0::libusb-1.0)
    set(LIBUSB_TARGET libusb-1.0::libusb-1.0)
    message(STATUS "libusb-1.0      : found via config mode")
else()
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(libusb QUIET IMPORTED_TARGET libusb-1.0)
    endif()
    if(TARGET PkgConfig::libusb)
        set(LIBUSB_TARGET PkgConfig::libusb)
        message(STATUS "libusb-1.0      : found via pkg-config")
    else()
        find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h PATH_SUFFIXES libusb-1.0)
        find_library(LIBUSB_LIBRARY NAMES usb-1.0 usb)
        if(NOT LIBUSB_INCLUDE_DIR OR NOT LIBUSB_LIBRARY)
            message(FATAL_ERROR
                    "libusb-1.0 not found. Tried config mode, pkg-config, and find_library.")
        endif()
        add_library(libusb-1.0::libusb-1.0 UNKNOWN IMPORTED)
        set_target_properties(libusb-1.0::libusb-1.0 PROPERTIES
                IMPORTED_LOCATION             "${LIBUSB_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBUSB_INCLUDE_DIR}"
        )
        set(LIBUSB_TARGET libusb-1.0::libusb-1.0)
        message(STATUS "libusb-1.0      : found via find_library (${LIBUSB_LIBRARY})")
    endif()
endif()

# ─── FFMPEG ───────────────────────────────────────────────────────────────────
find_package(FFMPEG REQUIRED)

# ─── taglib ───────────────────────────────────────────────────────────────────
find_package(taglib REQUIRED)

# ─── Linux-only dependencies ──────────────────────────────────────────────────
if(OPENAUTO_LINUX)
    message(STATUS "Linux: searching for blkid and gps")
    find_package(blkid REQUIRED)
    find_package(gps   REQUIRED)
endif()

# ─── Windows-specific ─────────────────────────────────────────────────────────
if(OPENAUTO_WINDOWS)
    set(PLATFORM_LINK_LIBS ws2_32)
endif()

# ─── Qt Design Studio components (network fetch) ─────────────────────────────
# qmlcomponents.cmake uses FetchContent to download from GitHub at configure
# time. This works on desktop but will fail in Yocto's network-isolated build.
# Automatically disabled when cross-compiling; can be forced off with
# -DOPENAUTO_QDS_COMPONENTS=OFF on any platform.
option(OPENAUTO_QDS_COMPONENTS "Fetch and build Qt Design Studio components (requires network)" ON)
if(OPENAUTO_QDS_COMPONENTS AND CMAKE_CROSSCOMPILING)
    message(STATUS "QDS components  : DISABLED (cross-compiling — no network access)")
    set(OPENAUTO_QDS_COMPONENTS OFF)
elseif(OPENAUTO_QDS_COMPONENTS)
    message(STATUS "QDS components  : enabled")
else()
    message(STATUS "QDS components  : disabled")
endif()

# ─── Source files ─────────────────────────────────────────────────────────────
set(sources_directory ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(include_directory ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(common_include_directory  ${include_directory}/f1x/openauto/Common)
set(config_include_directory  ${include_directory}/f1x/openauto/Configuration)
set(config_sources_directory  ${sources_directory}/Configuration)
set(autoapp_sources_directory ${sources_directory}/autoapp)
set(autoapp_include_directory ${include_directory}/f1x/openauto/autoapp)

file(GLOB_RECURSE autoapp_source_files
        CONFIGURE_DEPENDS
        ${autoapp_sources_directory}/*.ui      # required for AUTOUIC
        ${autoapp_sources_directory}/*.cpp
        ${autoapp_include_directory}/*.hpp     # required for AUTOMOC
        ${autoapp_include_directory}/*.h       # required for AUTOMOC
        ${config_sources_directory}/*.cpp
        ${config_include_directory}/*.hpp      # required for AUTOMOC
        ${common_include_directory}/*.hpp      # required for AUTOMOC
)

# Exclude journeyos-hardware-detect sources from the autoapp build.
# These live in Hardware/Detect/ and have their own main() entry point.
list(FILTER autoapp_source_files EXCLUDE REGEX ".*/Hardware/Detect/.*\\.cpp$")

# ─── Executable target ────────────────────────────────────────────────────────
qt_add_executable(autoapp ${autoapp_source_files})

# ─── Compiler options (cross-platform) ───────────────────────────────────────
target_compile_features(autoapp PRIVATE cxx_std_17)

# Semicolons required as list separators inside generator expressions —
# spaces silently fail with Xcode and Visual Studio generators.
target_compile_options(autoapp PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-pedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_compile_definitions(autoapp PRIVATE
        QT_MESSAGELOGCONTEXT
        BOOST_ALL_DYN_LINK
        $<$<CONFIG:Release>:NDEBUG>
        $<$<BOOL:${OPENAUTO_MACOS}>:MAC_OS>    # was add_definitions(-DMAC_OS) — now target-scoped
        OPENAUTO_VERSION_STRING="${LIBRARY_BUILD_MAJOR_RELEASE}.${LIBRARY_BUILD_MINOR_RELEASE}.${LIBRARY_BUILD_INCREMENTAL}"
)

# ─── Qt Design Studio subdirectories ─────────────────────────────────────────
set(UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/autoapp/UI/JourneyOS")
add_subdirectory(${UI_DIR}/JourneyOSContent)
add_subdirectory(${UI_DIR}/JourneyOS)

set_source_files_properties("${UI_DIR}/qtquickcontrols2.conf"
        PROPERTIES QT_RESOURCE_ALIAS "/qtquickcontrols2.conf"
)
qt_add_resources(autoapp "configuration"
        PREFIX "/"
        FILES  "${UI_DIR}/qtquickcontrols2.conf"
)

# ─── QDS component plugins ────────────────────────────────────────────────────
if(OPENAUTO_QDS_COMPONENTS)
    include(qmlcomponents)   # fetches QuickStudio* plugins from GitHub
endif()

# ─── Include directories (target-scoped) ─────────────────────────────────────
target_include_directories(autoapp PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${FFMPEG_INCLUDE_DIRS}
        ${TAGLIB_INCLUDE_DIRS}
)

# ─── Link libraries ───────────────────────────────────────────────────────────
# autoapp is an executable — PRIVATE throughout is correct.
# PUBLIC would leak dependencies transitively, which is meaningless for an exe.
target_link_libraries(autoapp PRIVATE
        # Android Auto stack
        AASDK::aasdk
        AAPProto::aap_protobuf

        # Abseil — required by protobuf 5.x headers (ABSL_CHECK / ABSL_LOG).
        # aap_protobuf's cmake config omits these transitive deps, so we link directly.
        absl::log
        absl::check

        # Qt modules
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::QuickEffects
        Qt6::Qml
        Qt6::DBus
        Qt6::Bluetooth
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        Qt6::Network
        Qt6::Widgets

        # Qt Design Studio UI plugins
        JourneyOSplugin
        JourneyOSContentplugin

        # System
        ${LIBUSB_TARGET}
        Boost::system
        Boost::log
        OpenSSL::SSL
        OpenSSL::Crypto

        # Media
        ${FFMPEG_LIBRARIES}
        ${TAGLIB_LIBRARIES}

        # Platform (Winsock on Windows, empty elsewhere)
        ${PLATFORM_LINK_LIBS}
)

# ─── Linux-only links ─────────────────────────────────────────────────────────
if(OPENAUTO_LINUX)
    target_include_directories(autoapp PRIVATE
            ${BLKID_INCLUDE_DIRS}
            ${GPS_INCLUDE_DIRS}
    )
    target_link_libraries(autoapp PRIVATE
            pulse
            ${BLKID_LIBRARIES}
            ${GPS_LIBRARIES}
    )
endif()

# ─── Output properties ────────────────────────────────────────────────────────
# VERSION/SOVERSION are for shared libraries, not executables.
# Use OUTPUT_NAME and RUNTIME_OUTPUT_DIRECTORY only.
set_target_properties(autoapp PROPERTIES
        OUTPUT_NAME              "autoapp"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
)

# ─── Install ──────────────────────────────────────────────────────────────────
install(TARGETS autoapp
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

# ─── Hardware detection tool ──────────────────────────────────────────────────
# Standalone Type=oneshot executable that runs once at boot (before Weston/autoapp)
# to detect hardware and write /run/journeyos/{hardware.json,kms.json,qt.env}.
set(hw_detect_sources
    src/autoapp/Hardware/Detect/main.cpp
    src/autoapp/Hardware/Detect/HardwareDetector.cpp
    src/autoapp/Hardware/Detect/QtHardwareDetector.cpp
)

if(OPENAUTO_LINUX)
    list(APPEND hw_detect_sources
        src/autoapp/Hardware/Detect/RpiHardwareDetector.cpp)
endif()

add_executable(journeyos-hardware-detect ${hw_detect_sources})

target_compile_features(journeyos-hardware-detect PRIVATE cxx_std_17)

target_compile_options(journeyos-hardware-detect PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-pedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_include_directories(journeyos-hardware-detect PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(journeyos-hardware-detect PRIVATE
    Qt6::Core
    Qt6::Gui          # QGuiApplication, QScreen
    Qt6::Network      # QNetworkInterface
    Qt6::Multimedia   # QMediaDevices, QAudioDevice, QCameraDevice
    Qt6::Bluetooth    # QBluetoothLocalDevice
)

# Enable RPi-specific detection code path on Linux builds
if(OPENAUTO_LINUX)
    target_compile_definitions(journeyos-hardware-detect PRIVATE
        JOURNEYOS_RPI_DETECTION)
endif()

set_target_properties(journeyos-hardware-detect PROPERTIES
    OUTPUT_NAME              "journeyos-hardware-detect"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_SBINDIR}"
)

install(TARGETS journeyos-hardware-detect
    RUNTIME DESTINATION "${CMAKE_INSTALL_SBINDIR}"
)