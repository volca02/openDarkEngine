#    This file is part of openDarkEngine project
#    Copyright (C) 2005-2009 openDarkEngine team
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

# TODO: set version and other properties directly in project statement
SET(OPDE_VER_MAJOR "0")
SET(OPDE_VER_MINOR "3")
SET(OPDE_VER_PATCH "0")
SET(REL_CODE_NAME  "El Motor Oscuro")

# for installation - directories with version in them (/usr/share/... etc.)
SET(OPDE_SHORTNAME_WITH_VERSION "${CMAKE_PROJECT_NAME}-${OPDE_VER_MAJOR}.${OPDE_VER_MINOR}")
MARK_AS_ADVANCED(OPDE_SHORTNAME_WITH_VERSION)

# Use C++11
set (CMAKE_CXX_STANDARD 11)

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

# Option to create Windows installer script
OPTION(GENERATE_NSIS
    "Creates Windows installer script"
    ON)

# Option to generate documentation
OPTION(GENERATE_DOC
    "Generate documentation"
    OFF)

# Option to build in the debugging code
OPTION(FRAME_PROFILER
    "Build in support for frame profiling (will log frame profiling events)"
    OFF)

# Option to build all the libs as static (defunct - linking problems)
# OPTION(STATIC_LIBS
#    "Build the libs as static (defunct)"
#    OFF)
SET(STATIC_LIBS OFF)
MARK_AS_ADVANCED(STATIC_LIBS)

# Option to build the python module/extension
OPTION(PYTHON_MODULE
    "Build the python module version of the OpenDark library"
    OFF)

# Installation directories
# SET(OPDE_BINARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
SET(OPDE_BINARY_INSTALL_DIR "bin")
MARK_AS_ADVANCED(OPDE_BINARY_INSTALL_DIR)
# SET(OPDE_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
# TODO: lib64?
SET(OPDE_LIBRARY_INSTALL_DIR "lib")
MARK_AS_ADVANCED(OPDE_LIBRARY_INSTALL_DIR)

SET(OPDE_DATA_INSTALL_DIR share/${OPDE_SHORTNAME_WITH_VERSION})
SET(OPDE_FULL_DATA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${OPDE_DATA_INSTALL_DIR})
MARK_AS_ADVANCED(OPDE_DATA_INSTALL_DIR)
MARK_AS_ADVANCED(OPDE_FULL_DATA_INSTALL_DIR)

#Target the binary files into a separate directories
# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR/Build})

# MinGW specific
IF(CMAKE_COMPILER_IS_MINGW)
    SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
    SET(CMAKE_SHARED_LINKER_FLAGS"-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
ENDIF(CMAKE_COMPILER_IS_MINGW)

# Warning level high (Lowered by one to supress the payload of ogre related warnings in VC)
SET(CMAKE_CXX_WARNING_LEVEL 3)

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
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Global debug option test. Sets the DEBUG flag in the config.h
IF(GLOBAL_DEBUG)
    MESSAGE(STATUS "Debugging is ON")
    SET(OPDE_DEBUG 1)
ELSE(GLOBAL_DEBUG)
    MESSAGE(STATUS "Debugging is Off")
ENDIF(GLOBAL_DEBUG)


# Solve the dependencies
SET(ODE_DIR ${CMAKE_MODULE_PATH})
SET(OGRE_DIR ${CMAKE_MODULE_PATH})
SET(FREEIMAGE_DIR ${CMAKE_MODULE_PATH})

FIND_PACKAGE(ODE REQUIRED)
FIND_PACKAGE(OGRE REQUIRED)
FIND_PACKAGE(FREEIMAGE REQUIRED)
FIND_PACKAGE(SDL2 REQUIRED)

# TODO: REMOVE, TEMPORARY TILL WE CHANGE TO OGRE's STRICT RESOURCE MANAGER
PKG_CHECK_MODULES(ZZIPLIB REQUIRED zziplib)

# Documentation
# On Ubuntu, the following command does not search for Dot :(
FIND_PACKAGE(Doxygen)
FIND_PROGRAM(TEXI2HTML texi2html)

IF(NOT DOT)
	FIND_PROGRAM(DOT dot)
ENDIF(NOT DOT)

MARK_AS_ADVANCED(TEXI2HTML DOT)

INCLUDE(FindPythonModule.cmake)
FIND_PYTHON_MODULE("epydoc" "EPYDOC")

IF(FREEIMAGE_FOUND)
    MESSAGE(STATUS "FreeImage Found OK")
    MESSAGE(STATUS " - FreeImage includes ${FREEIMAGE_INCLUDE_DIR}")
    MESSAGE(STATUS " - FreeImage libs ${FREEIMAGE_LIBRARIES}")
ELSE(FREEIMAGE_FOUND)
    MESSAGE(FATAL_ERROR "FreeImage not found and selected in options!")
ENDIF(FREEIMAGE_FOUND)

IF(ODE_FOUND)
    MESSAGE(STATUS "ODE Found OK")
    MESSAGE(STATUS " - ODE includes ${ODE_INCLUDE_DIR}")
    MESSAGE(STATUS " - ODE libs ${ODE_LIBRARIES}")
ELSE(ODE_FOUND)
    MESSAGE(FATAL_ERROR "ODE not found and selected in options!")
ENDIF(ODE_FOUND)

IF(SDL2_FOUND)
    MESSAGE(STATUS "SDL2 Found OK")
    MESSAGE(STATUS " - SDL2 includes ${SDL2_INCLUDE_DIR}")
    MESSAGE(STATUS " - SDL2 libs ${SDL2_LIBRARIES}")
ELSE(SDL2_FOUND)
    MESSAGE(FATAL_ERROR "SDL2 not found!")
ENDIF(SDL2_FOUND)

FIND_PACKAGE(PythonLibs REQUIRED)
SET(OPDE_PYTHON_LIBRARIES ${PYTHON_LIBRARIES})

# Aditional libraries - deps of the python lib
IF(UNIX)
    INCLUDE(PythonDeps.cmake)
    FIND_PYTHON_DEPS(OPDE_PYTHON_LIBRARIES)
ENDIF(UNIX)

IF(OPDE_PYTHON_LIBRARIES)
    MESSAGE(STATUS "Python Found OK")
    MESSAGE(STATUS " - Python includes ${PYTHON_INCLUDE_PATH}")
    MESSAGE(STATUS " - Python libs ${OPDE_PYTHON_LIBRARIES}")
ELSE(OPDE_PYTHON_LIBRARIES)
    MESSAGE(FATAL_ERROR "Python not found")
ENDIF(OPDE_PYTHON_LIBRARIES)

# Only visual check enabling verbose output, and package found info
IF(OGRE_FOUND)
    MESSAGE(STATUS "OGRE Found OK")
    MESSAGE(STATUS " - Ogre includes ${OGRE_INCLUDE_DIR}")
    MESSAGE(STATUS " - Ogre libs ${OGRE_LIBRARIES}")
ELSE(OGRE_FOUND)
    MESSAGE(FATAL_ERROR "OGRE not found")
ENDIF(OGRE_FOUND)

SET(DOC_DEPS_SATISFIED OFF)
MARK_AS_ADVANCED(DOC_DEPS_SATISFIED)

# Preparation for the documentation generators - do we have all dependencies?
IF (GENERATE_DOC)
    IF(DOXYGEN_EXECUTABLE)
        MESSAGE(STATUS "Doxygen Found OK")
    ELSE(DOXYGEN_EXECUTABLE)
        MESSAGE("Doxygen not found")
    ENDIF(DOXYGEN_EXECUTABLE)

    IF(TEXI2HTML)
        MESSAGE(STATUS "Texi2html Found OK")
    ELSE(TEXI2HTML)
        MESSAGE("Texi2html not found")
    ENDIF(TEXI2HTML)

    IF(DOT)
        MESSAGE(STATUS "Dot Found OK")
    ELSE(DOT)
        MESSAGE("Dot not found")
    ENDIF(DOT)

    IF(EPYDOC)
        MESSAGE(STATUS "Epydoc Found OK")
    ELSE(EPYDOC)
        MESSAGE("Epydoc not found")
    ENDIF(EPYDOC)

    # See if we have all the dependencies for DOC_DEPS_SATISFIED
    IF (DOXYGEN_EXECUTABLE AND TEXI2HTML AND DOT AND EPYDOC)
        SET(DOC_DEPS_SATISFIED ON)
    ELSE (DOXYGEN_EXECUTABLE AND TEXI2HTML AND DOT AND EPYDOC)
        MESSAGE("Warning: Documentation dependencies not satisfied. Doc generation will be disabled.")
        SET(GENERATE_DOC OFF)
    ENDIF (DOXYGEN_EXECUTABLE AND TEXI2HTML AND DOT AND EPYDOC)
ENDIF (GENERATE_DOC)

# Some configuration checks to allow platform independence
INCLUDE(ConfigureChecks.cmake)
CONFIGURE_FILE(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )

# Add the subdirectories which contain aditional CMakeLists.txt files
add_subdirectory (src)
add_subdirectory (proto)
add_subdirectory (thirdparty)

# TODO: We have to redo the doc generation as a custom target

# Additional fake_install dependencies
SET(OPDE_DOC_INST_DEPS "")
MARK_AS_ADVANCED(OPDE_DOC_INST_DEPS)

# Generate the doxyfile if selected as an option
# TODO: This will most certainly fail on windows due to the fact we use unix commands.
IF (GENERATE_DOC AND DOC_DEPS_SATISFIED)
    SET(OPDE_DOC_DIR share/${OPDE_SHORTNAME_WITH_VERSION})

    # Generate the doxyfile from the template file
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                      ${CMAKE_BINARY_DIR}/Doxyfile)

    # Now prepare the python doc. generator
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/writedoc.py.in
                      ${CMAKE_BINARY_DIR}/writedoc.py)

    # doxygen make target
    IF (DOXYGEN_EXECUTABLE)
    	ADD_CUSTOM_TARGET(doc ALL
        	COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
		COMMENT "Building doxygen documentation" VERBATIM
#		SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in
	)
    ELSE (DOXYGEN_EXECUTABLE)
    	MESSAGE("Warning: Doxygen not found, docs target disabled.")
    ENDIF (DOXYGEN_EXECUTABLE)

    # Thanks Ogre guys for this trick...
	# Create a dirctory for the python docs...
    file(WRITE ${CMAKE_BINARY_DIR}/doc/python/html/temp.txt "This file should be deleted before installation. It is only here to force the creation of the python doc. folder")
    file(REMOVE ${CMAKE_BINARY_DIR}/doc/python/html/temp.txt)


    # generates the Python docs.
    ADD_CUSTOM_TARGET(pydoc ALL
    	DEPENDS opdeScript
        COMMAND ${CMAKE_BINARY_DIR}/src/main/opdeScript ${CMAKE_BINARY_DIR}/writedoc.py
	COMMENT "Building Python API documentation" VERBATIM
#	SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/writedoc.py.in
    )

    MARK_AS_ADVANCED(MANUAL_BUILDER)
    # on win32 we use the buildmanual.bat, otherwise we use the buildmanual.sh
    IF(WIN32)
    	SET(MANUAL_BUILDER "${CMAKE_CURRENT_SOURCE_DIR}/doc/src/buildmanual.bat")
    ELSE(WIN32)
       	SET(MANUAL_BUILDER "/bin/sh" "${CMAKE_CURRENT_SOURCE_DIR}/doc/src/buildmanual.sh")
    ENDIF(WIN32)

    # generates the Developers manual
    ADD_CUSTOM_TARGET(manual ALL
        COMMAND ${MANUAL_BUILDER} ${CMAKE_CURRENT_SOURCE_DIR}/doc/src/ ${CMAKE_BINARY_DIR}/doc/manual/
	COMMENT "Building developer's manual" VERBATIM
#	SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/doc/src/manual.texi
    )

    # Copy various common files (style.css so far)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doc/style.css
                      ${CMAKE_BINARY_DIR}/doc/style.css)

    # TODO: property/link docs (maybe part of the writedoc.py?)

    # Install directives for the documentation
    # TODO: Skip .md5 and .map files as those are doxygen temporary
    INSTALL(DIRECTORY ${OPDE_BINARY_DIR}/doc DESTINATION ${OPDE_DOC_DIR})

    # additional fake_install deps
    SET(OPDE_DOC_INST_DEPS "doc" "pydoc" "manual")
ENDIF (GENERATE_DOC AND DOC_DEPS_SATISFIED)

IF (GENERATE_NSIS)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/installer/opde-win.in
                      ${CMAKE_BINARY_DIR}/installer/opde-win.nsi @ONLY)
        MESSAGE(STATUS "Installer script was generated in ${CMAKE_BINARY_DIR}/installer.")
ENDIF (GENERATE_NSIS)

# Optional testing local install (not automatic, use "make fake_install")
ADD_CUSTOM_TARGET(fake_install
    "${CMAKE_COMMAND}"
    -D CMAKE_INSTALL_PREFIX:string=${CMAKE_CURRENT_BINARY_DIR}/fake_install/${CMAKE_INSTALL_PREFIX}
    -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
#    DEPENDS opdeMain OpenDark ${OPDE_DOC_INST_DEPS}
)

MESSAGE(STATUS "Your configuration seems to be OK!")

MESSAGE(STATUS "")

# Print out some configuration status messages
MESSAGE(STATUS "Your build parameters:")
MESSAGE(STATUS " * Global debugging : ${GLOBAL_DEBUG}")
MESSAGE(STATUS " * Build type       : ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS " * Python module    : ${PYTHON_MODULE}")
MESSAGE(STATUS " * Documentation    : ${GENERATE_DOC}")
MESSAGE(STATUS " * C compiler       : ${CMAKE_C_COMPILER}")
MESSAGE(STATUS " * C++ compiler     : ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS " * Source directory : ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(STATUS " * Binary directory : ${CMAKE_CURRENT_BINARY_DIR}")
