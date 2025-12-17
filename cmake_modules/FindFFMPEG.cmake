# cmake_modules/FindFFMPEG.cmake

# Define the components we need
set(FFMPEG_COMPONENTS avcodec avutil swscale)

# 1. Setup Search Paths (Prioritize your custom build)
set(FFMPEG_SEARCH_PATHS
        "/usr/local/ffmpeg"         # Your manual install
        "/opt/homebrew/opt/ffmpeg"  # Mac Apple Silicon
        "/usr/local/opt/ffmpeg"     # Mac Intel
        "/usr/local"
        "/usr"
)

# 2. Find Headers and Libraries for each component
foreach(COMPONENT ${FFMPEG_COMPONENTS})
    string(TOUPPER ${COMPONENT} COMPONENT_UPPER)

    # Find Header (e.g., libavcodec/avcodec.h)
    find_path(FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR
            NAMES lib${COMPONENT}/${COMPONENT}.h
            PATHS ${FFMPEG_SEARCH_PATHS}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH
    )
    # Fallback to system paths if not found in custom locations
    find_path(FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR
            NAMES lib${COMPONENT}/${COMPONENT}.h
            PATH_SUFFIXES include
    )

    # Find Library (e.g., libavcodec.a or libavcodec.dylib)
    find_library(FFMPEG_${COMPONENT_UPPER}_LIBRARY
            NAMES ${COMPONENT}
            PATHS ${FFMPEG_SEARCH_PATHS}
            PATH_SUFFIXES lib
            NO_DEFAULT_PATH
    )
    # Fallback
    find_library(FFMPEG_${COMPONENT_UPPER}_LIBRARY
            NAMES ${COMPONENT}
            PATH_SUFFIXES lib
    )

    if (FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR AND FFMPEG_${COMPONENT_UPPER}_LIBRARY)
        set(FFMPEG_${COMPONENT_UPPER}_FOUND TRUE)
        list(APPEND FFMPEG_INCLUDE_DIRS ${FFMPEG_${COMPONENT_UPPER}_INCLUDE_DIR})
        list(APPEND FFMPEG_LIBRARIES ${FFMPEG_${COMPONENT_UPPER}_LIBRARY})
    else()
        set(FFMPEG_${COMPONENT_UPPER}_FOUND FALSE)
    endif()
endforeach()

# 3. Handle macOS Framework dependencies (Required for static FFmpeg builds)
if (APPLE)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)
    find_library(VIDEO_TOOLBOX_FRAMEWORK VideoToolbox)
    find_library(AUDIO_TOOLBOX_FRAMEWORK AudioToolbox)
    find_library(SECURITY_FRAMEWORK Security)

    # Compression libs often needed by static builds
    find_library(ICONV_LIBRARY iconv)
    find_library(ZLIB_LIBRARY z)
    find_library(BZ2_LIBRARY bz2)

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

# 4. Standard Boilerplate to handle "REQUIRED" arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG
        REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIRS
        HANDLE_COMPONENTS
)

# 5. Hide variables from GUI
mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)