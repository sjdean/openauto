#
#  This file is part of openauto project.
#  Copyright (C) 2018 f1x.studio (Michal Szwaj)
#
#  openauto is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.

#  openauto is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with openauto. If not, see <http://www.gnu.org/licenses/>.
#

if (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIRS)
  # in cache already
  set(TAGLIB_FOUND TRUE)
else (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIRS)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(TAGLIB QUIET taglib)
    if(TAGLIB_FOUND)
      message(STATUS "taglib found via pkg-config")
      return()
    endif()
  endif()

  find_path(TAGLIB_INCLUDE_DIR
    NAMES
        taglib.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
	PATH_SUFFIXES
          taglib
  )

  find_library(TAGLIB_LIBRARY
    NAMES
      tag taglib libtag
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(TAGLIB_INCLUDE_DIRS
    ${TAGLIB_INCLUDE_DIR}
  )
  set(TAGLIB_LIBRARIES
    ${TAGLIB_LIBRARY}
)

  if (TAGLIB_INCLUDE_DIRS AND TAGLIB_LIBRARIES)
     set(TAGLIB_FOUND TRUE)
  endif (TAGLIB_INCLUDE_DIRS AND TAGLIB_LIBRARIES)

  if (TAGLIB_FOUND)
    if (NOT taglib_FIND_QUIETLY)
      message(STATUS "Found taglib:")
          message(STATUS " - Includes: ${TAGLIB_INCLUDE_DIRS}")
          message(STATUS " - Libraries: ${TAGLIB_LIBRARIES}")
    endif (NOT taglib_FIND_QUIETLY)
  else (TAGLIB_FOUND)
    if (taglib_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find taglib")
    endif (taglib_FIND_REQUIRED)
  endif (TAGLIB_FOUND)

    mark_as_advanced(TAGLIB_INCLUDE_DIRS TAGLIB_LIBRARIES)

endif (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIRS)
