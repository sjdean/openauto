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

if (BLKID_LIBRARIES AND BLKID_INCLUDE_DIRS)
  # in cache already
  set(BLKID_FOUND TRUE)
else (BLKID_LIBRARIES AND BLKID_INCLUDE_DIRS)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(BLKID QUIET blkid)
    if(BLKID_FOUND)
      message(STATUS "blkid found via pkg-config")
      return()
    endif()
  endif()

  find_path(BLKID_INCLUDE_DIR
    NAMES
        blkid.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
	PATH_SUFFIXES
          blkid
  )

  find_library(BLKID_LIBRARY
    NAMES
      blkid
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(BLKID_INCLUDE_DIRS
    ${BLKID_INCLUDE_DIR}
  )
  set(BLKID_LIBRARIES
    ${BLKID_LIBRARY}
)

  if (BLKID_INCLUDE_DIRS AND BLKID_LIBRARIES)
     set(BLKID_FOUND TRUE)
  endif (BLKID_INCLUDE_DIRS AND BLKID_LIBRARIES)

  if (BLKID_FOUND)
    if (NOT blkid_FIND_QUIETLY)
      message(STATUS "Found blkid:")
          message(STATUS " - Includes: ${BLKID_INCLUDE_DIRS}")
          message(STATUS " - Libraries: ${BLKID_LIBRARIES}")
    endif (NOT blkid_FIND_QUIETLY)
  else (BLKID_FOUND)
    if (blkid_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find blkid")
    endif (blkid_FIND_REQUIRED)
  endif (BLKID_FOUND)

    mark_as_advanced(BLKID_INCLUDE_DIRS BLKID_LIBRARIES)

endif (BLKID_LIBRARIES AND BLKID_INCLUDE_DIRS)
