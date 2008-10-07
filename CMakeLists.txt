#    This file is part of openDarkEngine project
#    Copyright (C) 2005-2007 openDarkEngine team
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    $Id$

cmake_minimum_required(VERSION 2.6)

project (OPDE)

SET(OPDE_VER_MAJOR "0")
SET(OPDE_VER_MINOR "2")
SET(OPDE_VER_PATCH "1")

# Build type selection. Use ccmake for selection of this (or the commandline switches)
# Default build type
IF (NOT CMAKE_BUILD_TYPE)
    # Release for now, as VC builds are incredibly slow on Debug
    SET(CMAKE_BUILD_TYPE Release)
ENDIF (NOT CMAKE_BUILD_TYPE)

# Option to build in the debugging code
OPTION(GLOBAL_DEBUG 
    "Build the debugging code"
    OFF)

# Option to build the documentation
OPTION(GENERATE_DOC
    "Build the doxy documentation"
    OFF)

# Option to create Windows installer script
OPTION(GENERATE_NSIS
    "Creates Windows installer script"
    ON)
    
# Option to build in the debugging code
OPTION(PROFILING 
    "Build the sources with profiling support (Only Debug build type)"
    OFF)

# Option to build all the libs as static
OPTION(STATIC_LIBS
    "Build the libs as static"
    OFF)


# Compiler profiling flags
IF (PROFILING)
    IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(PROFILER "-pg") # GCC only profiler option
    ENDIF(CMAKE_COMPILER_IS_GNUCXX)	
ELSE (PROFILING)
    SET(PROFILER "")
ENDIF (PROFILING)

#Target the binary files into a separate directories
# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR/Build})

# MinGW specific
IF(CMAKE_COMPILER_IS_MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
	SET(CMAKE_SHARED_LINKER_FLAGS"-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
ENDIF(CMAKE_COMPILER_IS_MINGW)

# Warning level high (Lowered by one to supress the payload of ogre related warnings in VC)
SET(CMAKE_CXX_WARNING_LEVEL 3)

# GCC specific. Release/Debug flags (I add some debug/profiler switches there)
IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_C_FLAGS_DEBUG "-g -O3 ${PROFILER}")
	SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3 ${PROFILER}")
	SET(CMAKE_CXX_FLAGS_DISTRIBUTION "-O3")
	SET(CMAKE_C_FLAGS_DISTRIBUTION "-O3")
	
	# extra warnings, but no unused params (too common)
	# no strict aliasing (see Py_True type punned pointer aliasing errors)
	ADD_DEFINITIONS(-Wall -fno-strict-aliasing -fvisibility=default)
	#  -Wextra -Wno-unused-parameter
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# -fPIC for 64bit IA (x86_64 that is)
IF(UNIX AND NOT WIN32)
  FIND_PROGRAM(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin )
  IF(CMAKE_UNAME)
    EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
    SET(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE INTERNAL
"processor type (i386 and x86_64)")
    IF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
      ADD_DEFINITIONS(-fPIC)
    ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  ENDIF(CMAKE_UNAME)
ENDIF(UNIX AND NOT WIN32)


# MSVC specific stuff
IF(CMAKE_COMPILER_IS_MSVC)
	# STUB. Nothing now
ENDIF(CMAKE_COMPILER_IS_MSVC)

# Various modules used for library path detections.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Global debug option test. Sets the DEBUG flag in the config.h
IF(GLOBAL_DEBUG)
    MESSAGE(STATUS "Debugging is ON")
    SET(OPDE_DEBUG 1)
ELSE(GLOBAL_DEBUG)
    MESSAGE(STATUS "Debugging is Off")
ENDIF(GLOBAL_DEBUG)



# Solve the dependencies
SET(OIS_DIR ${CMAKE_MODULE_PATH})
SET(OGRE_DIR ${CMAKE_MODULE_PATH})
SET(FREEIMAGE_DIR ${CMAKE_MODULE_PATH})

FIND_PACKAGE(OIS REQUIRED)
FIND_PACKAGE(OGRE REQUIRED)

# Custom image hooks
FIND_PACKAGE(FREEIMAGE REQUIRED)

IF(FREEIMAGE_FOUND)
	MESSAGE(STATUS "FreeImage Found OK")
	MESSAGE(STATUS " - FreeImage includes ${FREEIMAGE_INCLUDE_DIR}")
	MESSAGE(STATUS " - FreeImage libs ${FREEIMAGE_LIBRARIES}")
ELSE(FREEIMAGE_FOUND)
	MESSAGE(FATAL_ERROR "FreeImage not found and selected in options!")
ENDIF(FREEIMAGE_FOUND)


FIND_PACKAGE(PythonLibs REQUIRED)
    
# Aditional libraries - deps of the python lib
IF(UNIX)
	INCLUDE(PythonDeps.cmake)
	FIND_PYTHON_DEPS(PYTHON_LIBRARIES)
ENDIF(UNIX)

IF(PYTHON_LIBRARIES)
	MESSAGE(STATUS "Python Found OK")
	MESSAGE(STATUS " - Python includes ${PYTHON_INCLUDE_PATH}")
	MESSAGE(STATUS " - Python libs ${PYTHON_LIBRARIES}")
ELSE(PYTHON_LIBRARIES)
	MESSAGE(FATAL_ERROR "Python not found and selected in options!")
ENDIF(PYTHON_LIBRARIES)


# Only visual check enabling verbose output, and package found info
IF(OIS_FOUND)
     MESSAGE(STATUS "OIS Found OK")
     MESSAGE(STATUS " - OIS includes ${OIS_INCLUDE_DIR}")
     MESSAGE(STATUS " - OIS libs ${OIS_LIBRARIES}")
ELSE(OIS_FOUND)
     MESSAGE(FATAL_ERROR "OIS not found")
ENDIF(OIS_FOUND)

IF(OGRE_FOUND)
    MESSAGE(STATUS "OGRE Found OK")
    MESSAGE(STATUS " - Ogre includes ${OGRE_INCLUDE_DIR}")
    MESSAGE(STATUS " - Ogre libs ${OGRE_LIBRARIES}")
ELSE(OGRE_FOUND)
    MESSAGE(FATAL_ERROR "OGRE not found")
ENDIF(OGRE_FOUND)

#IF(CEGUI_FOUND)
#    MESSAGE(STATUS "CEGUI Found OK")
#    MESSAGE(STATUS " - CEGUI includes ${CEGUI_INCLUDE_DIR}")
#    MESSAGE(STATUS " - CEGUI libs ${CEGUI_LIBRARIES}")
#ELSE(CEGUI_FOUND)
#    MESSAGE(FATAL_ERROR "CEGUI not found")
#ENDIF(CEGUI_FOUND)

# Some configuration checks to allow platform independence
INCLUDE(ConfigureChecks.cmake)
CONFIGURE_FILE(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )

# Add the subdirectories which contain aditional CMakeLists.txt files
add_subdirectory (src)
add_subdirectory (proto)
add_subdirectory (thirdparty)

# Generate the doxyfile if selected as an option
IF (GENERATE_DOC)
    # Generate the doxyfile from the template file
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                      ${CMAKE_BINARY_DIR}/Doxyfile)
    
    MESSAGE(STATUS "Doxyfile was generated in ${CMAKE_BINARY_DIR}.")
    # You can run doxygen on that file to generate documentation...
ENDIF (GENERATE_DOC)

IF (GENERATE_NSIS)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/installer/opde-win.in
                      ${CMAKE_BINARY_DIR}/installer/opde-win.nsi @ONLY)
        MESSAGE(STATUS "Installer script was generated in ${CMAKE_BINARY_DIR}/installer.")
ENDIF (GENERATE_NSIS)        

MESSAGE(STATUS "Your configuration seems to be OK!")

MESSAGE(STATUS "")

# Print out some configuration status messages
MESSAGE(STATUS "Your build parameters:")
MESSAGE(STATUS " * Global debugging : ${GLOBAL_DEBUG}")
MESSAGE(STATUS " * Build type       : ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS " * Profilling       : ${PROFILING}")
MESSAGE(STATUS " * Doxy generation  : ${GENERATE_DOC}")
MESSAGE(STATUS " * Custom image h.  : ${CUSTOM_IMAGE_HOOKS}")
MESSAGE(STATUS " * Static geometry  : ${__STATIC_GEOMETRY}")
MESSAGE(STATUS " * C compiler       : ${CMAKE_C_COMPILER}")
MESSAGE(STATUS " * C++ compiler     : ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS " * Source directory : ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(STATUS " * Binary directory : ${CMAKE_CURRENT_BINARY_DIR}")

