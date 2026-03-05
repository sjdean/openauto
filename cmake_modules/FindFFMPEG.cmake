# cmake_modules/FindFFMPEG.cmake

# Define the components we need
set(FFMPEG_COMPONENTS avcodec avutil swscale)

# Search paths for local/desktop installs.
# In Yocto these are ignored — CMake searches the sysroot automatically.
set(FFMPEG_SEARCH_PATHS
        "/usr/local/ffmpeg"         # Manual install
        "/opt/homebrew/opt/ffmpeg"  # Mac Apple Silicon
        "/usr/local/opt/ffmpeg"     # Mac Intel
        "/usr/local"
        "/usr"
)

# ─── Find headers and libraries for each component ───────────────────────────
foreach(COMPONENT ${FFMPEG_COMPONENTS})
    string(TOUPPER ${COMPONENT} COMPONENT_UPPER)

    # Pass 1: custom install locations (desktop/Mac — NO_DEFAULT_PATH means
    #         CMake won't search the sysroot here, so Yocto skips to pass 3)
    find_path(FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR
            NAMES lib${COMPONENT}/${COMPONENT}.h
            PATHS ${FFMPEG_SEARCH_PATHS}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH
    )
    find_library(FFMPEG_${COMPONENT_UPPER}_LIBRARY
            NAMES ${COMPONENT}
            PATHS ${FFMPEG_SEARCH_PATHS}
            PATH_SUFFIXES lib
            NO_DEFAULT_PATH
    )

    # Pass 2: system paths on desktop Linux (no custom paths, no sysroot restriction)
    if(NOT FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR)
        find_path(FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR
                NAMES lib${COMPONENT}/${COMPONENT}.h
                PATH_SUFFIXES include
        )
    endif()
    if(NOT FFMPEG_${COMPONENT_UPPER}_LIBRARY)
        find_library(FFMPEG_${COMPONENT_UPPER}_LIBRARY
                NAMES ${COMPONENT}
                PATH_SUFFIXES lib
        )
    endif()

    # Pass 3: Yocto sysroot — no PATHS, no PATH_SUFFIXES, no NO_DEFAULT_PATH.
    # CMake's CMAKE_FIND_ROOT_PATH mechanism (set by the Yocto toolchain file)
    # redirects these searches into recipe-sysroot automatically.
    if(NOT FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR)
        find_path(FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR
                NAMES lib${COMPONENT}/${COMPONENT}.h
        )
    endif()
    if(NOT FFMPEG_${COMPONENT_UPPER}_LIBRARY)
        find_library(FFMPEG_${COMPONENT_UPPER}_LIBRARY
                NAMES ${COMPONENT}
        )
    endif()

    # Accumulate results
    if(FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR AND FFMPEG_${COMPONENT_UPPER}_LIBRARY)
        set(FFMPEG_${COMPONENT_UPPER}_FOUND TRUE)
        list(APPEND FFMPEG_INCLUDE_DIRS ${FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR})
        list(APPEND FFMPEG_LIBRARIES    ${FFMPEG_${COMPONENT_UPPER}_LIBRARY})
        message(STATUS "FFMPEG ${COMPONENT} : ${FFMPEG_${COMPONENT_UPPER}_LIBRARY}")
    else()
        set(FFMPEG_${COMPONENT_UPPER}_FOUND FALSE)
        message(STATUS "FFMPEG ${COMPONENT} : NOT FOUND")
    endif()

endforeach()
# ─── End per-component search ─────────────────────────────────────────────────

# ─── macOS framework dependencies (required for static FFmpeg builds) ─────────
# Outside the foreach — these are platform-level deps, not per-component.
if(APPLE)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_VIDEO_FRAMEWORK      CoreVideo)
    find_library(VIDEO_TOOLBOX_FRAMEWORK   VideoToolbox)
    find_library(AUDIO_TOOLBOX_FRAMEWORK   AudioToolbox)
    find_library(SECURITY_FRAMEWORK        Security)
    find_library(ICONV_LIBRARY             iconv)
    find_library(ZLIB_LIBRARY              z)
    find_library(BZ2_LIBRARY               bz2)

    list(APPEND FFMPEG_LIBRARIES
            ${CORE_FOUNDATION_FRAMEWORK}
            ${CORE_VIDEO_FRAMEWORK}
            ${VIDEO_TOOLBOX_FRAMEWORK}
            ${AUDIO_TOOLBOX_FRAMEWORK}
            ${SECURITY_FRAMEWORK}
            ${ICONV_LIBRARY}
            ${ZLIB_LIBRARY}
            ${BZ2_LIBRARY}
    )
endif()

# ─── Deduplicate include dirs (multiple components may share a root) ──────────
if(FFMPEG_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)
endif()

# ─── Standard boilerplate — handles REQUIRED and QUIET arguments ──────────────
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG
        REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIRS
        HANDLE_COMPONENTS
)

mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)