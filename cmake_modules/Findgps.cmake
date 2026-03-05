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

if (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)
  # in cache already
  set(GPS_FOUND TRUE)
else (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(GPS QUIET gps)
    if(GPS_FOUND)
      message(STATUS "gps found via pkg-config")
      return()
    endif()
  endif()

  find_path(GPS_INCLUDE_DIR
    NAMES
        gps.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
	PATH_SUFFIXES
          gps
  )

  find_library(GPS_LIBRARY
    NAMES
      gps
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(GPS_INCLUDE_DIRS
    ${GPS_INCLUDE_DIR}
  )
  set(GPS_LIBRARIES
    ${GPS_LIBRARY}
)

  if (GPS_INCLUDE_DIRS AND GPS_LIBRARIES)
     set(GPS_FOUND TRUE)
  endif (GPS_INCLUDE_DIRS AND GPS_LIBRARIES)

  if (GPS_FOUND)
    if (NOT gps_FIND_QUIETLY)
      message(STATUS "Found gps:")
          message(STATUS " - Includes: ${GPS_INCLUDE_DIRS}")
          message(STATUS " - Libraries: ${GPS_LIBRARIES}")
    endif (NOT gps_FIND_QUIETLY)
  else (GPS_FOUND)
    if (gps_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find gps")
    endif (gps_FIND_REQUIRED)
  endif (GPS_FOUND)

    mark_as_advanced(GPS_INCLUDE_DIRS GPS_LIBRARIES)

endif (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)
