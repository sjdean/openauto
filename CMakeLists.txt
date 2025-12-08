cmake_minimum_required(VERSION 3.15)
project(openauto CXX)
message(STATUS "OpenAuto")

# --- Global Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
include(GNUInstallDirs)

# --- Automatic Versioning ---
include(gitversion)
set(OPENAUTO_BUILD_MAJOR_RELEASE 4)
set(OPENAUTO_BUILD_MINOR_RELEASE 1)
set(PROGRAM_VERSION_STRING "${OPENAUTO_BUILD_MAJOR_RELEASE}.${OPENAUTO_BUILD_MINOR_RELEASE}.${_git_commit_count}+${_build_version}")
message(STATUS "Project Version: ${PROGRAM_VERSION_STRING}")

# --- Build Type Configuration ---
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Forcing Release build type")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

# --- macOS Specifics ---
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    message(STATUS "MacOS System Detected")
    add_definitions(-DMAC_OS)
    find_package(protobuf REQUIRED CONFIG)
    find_package(absl REQUIRED)
else ()
    find_package(Protobuf REQUIRED) # Provides Protobuf::protobuf
endif ()

# --- Find Dependencies (Modern CONFIG Mode) ---
find_package(aap_protobuf REQUIRED CONFIG) # Finds config, provides AAPProto::aap_protobuf
find_package(aasdk REQUIRED CONFIG)        # Finds config, provides AASDK::aasdk
find_package(Boost REQUIRED COMPONENTS system log)
find_package(libusb-1.0 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Bluetooth DBus Network Multimedia MultimediaWidgets)
if(APPLE)
    message(STATUS "Patching Qt6::Gui to remove deprecated AGL framework dependency...")

    # Get the list of libraries Qt::Gui thinks it needs
    get_property(QT_GUI_LIBS TARGET Qt6::Gui PROPERTY INTERFACE_LINK_LIBRARIES)

    # Remove AGL from that list
    list(REMOVE_ITEM QT_GUI_LIBS "-framework AGL")

    # Set the list back without AGL
    set_property(TARGET Qt6::Gui PROPERTY INTERFACE_LINK_LIBRARIES "${QT_GUI_LIBS}")
endif()
find_package(OpenSSL REQUIRED)
find_package(taglib REQUIRED)

# --- Linux-Only Dependencies ---
if (LINUX)
    message(STATUS "Linux system detected, searching for Linux libraries")
    find_package(blkid REQUIRED)
    find_package(gps REQUIRED)
else ()
    message(STATUS "Non-Linux system, skipping linux only libraries")
endif ()

if (WIN32)
    set(WINSOCK2_LIBRARIES "ws2_32")
endif ()

# --- QML Configuration ---
#set(UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/autoapp/UI/JourneyOS")
#add_subdirectory(${UI_DIR})

set(UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/autoapp/UI/JourneyOS")

# Set Main Source Folders
set(sources_directory ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(include_directory ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Set common source
set(common_include_directory ${include_directory}/f1x/openauto/Common)
set(config_include_directory ${include_directory}/f1x/openauto/Configuration)
set(config_sources_directory ${sources_directory}/Configuration)

# Set autoapp sources
set(autoapp_sources_directory ${sources_directory}/autoapp)
set(autoapp_include_directory ${include_directory}/f1x/openauto/autoapp)

file(GLOB_RECURSE autoapp_source_files
        ${autoapp_sources_directory}/*.ui   # <-- MUST keep for AUTOUIC
        ${autoapp_sources_directory}/*.cpp
        ${autoapp_include_directory}/*.hpp # <-- MUST keep for AUTOMOC
        ${autoapp_include_directory}/*.h   # <-- MUST keep for AUTOMOC
        ${config_sources_directory}/*.cpp
        ${config_include_directory}/*.hpp # <-- MUST keep for AUTOMOC
        ${common_include_directory}/*.hpp) # <-- MUST keep for AUTOMOC

add_compile_definitions(QT_MESSAGELOGCONTEXT)

# --- Target: autoapp ---
qt_add_executable(autoapp ${autoapp_source_files})

target_compile_features(autoapp PRIVATE cxx_std_17)
target_compile_options(autoapp PRIVATE -Wall -pedantic)
target_compile_definitions(autoapp PRIVATE BOOST_ALL_DYN_LINK)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(autoapp PRIVATE NDEBUG)
endif ()

# ==============================================================
# JourneyOS UI – Qt Design Studio integration (this is the only part you needed)
# ==============================================================

set(UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/autoapp/UI/JourneyOS")

# Build the two QDS plugins
add_subdirectory(${UI_DIR}/JourneyOSContent)
add_subdirectory(${UI_DIR}/JourneyOS)


# This single line silences the Qt6CoreMacros error forever
set_source_files_properties("${UI_DIR}/qtquickcontrols2.conf"
        PROPERTIES QT_RESOURCE_ALIAS "/qtquickcontrols2.conf"
)

# The trick: use an absolute path BUT tell Qt explicitly that it's intentional
qt_add_resources(autoapp "configuration"
        PREFIX "/"
        FILES ${UI_DIR}/qtquickcontrols2.conf)


# Link the two plugins (you can list them once here – no need to repeat later)
target_link_libraries(autoapp PRIVATE
        JourneyOSplugin
        JourneyOSContentplugin
)

include(qmlcomponents)

# ==============================================================
# Rest of your original linking (unchanged, just without the duplicate plugin lines)
# ==============================================================

target_include_directories(autoapp PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TAGLIB_INCLUDE_DIRS}
        ${ILCLIENT_INCLUDE_DIRS}
)

target_link_libraries(autoapp PUBLIC
        AASDK::aasdk
        AAPProto::aap_protobuf
        Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml Qt6::DBus
        Qt6::Bluetooth Qt6::Multimedia Qt6::MultimediaWidgets Qt6::Network
        libusb
        PRIVATE
        Boost::log
        ${TAGLIB_LIBRARIES}
        ${ILCLIENT_LIBRARIES}
        ${WINSOCK2_LIBRARIES}
)

if (LINUX)
    target_include_directories(autoapp PRIVATE
            ${BLKID_INCLUDE_DIRS}
            ${GPS_INCLUDE_DIRS}
    )
    target_link_libraries(autoapp PRIVATE
            pulse
            ${BLKID_LIBRARIES}
            ${GPS_LIBRARIES}
    )
endif ()

set_target_properties(autoapp PROPERTIES
        VERSION ${PROGRAM_VERSION_STRING}
        SOVERSION ${_git_commit_count}
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
)

# --- Install Rules ---
install(TARGETS autoapp
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)