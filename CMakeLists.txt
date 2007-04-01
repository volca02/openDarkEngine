project (OPDE)

# Build type selection. This should be targettable 
#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_BUILD_TYPE Release)

# Option to build in the debugging code
OPTION(GLOBAL_DEBUG 
    "Build the debugging code too"
    OFF)

# Option to build in the debugging code
OPTION(PROFILING 
    "Build the sources with profiling support (Only Debug build type)"
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

# Warning level high (Lowered by one to supress the payload of ogre releted warnings in VC)
SET(CMAKE_CXX_WARNING_LEVEL 3)

# GCC specific. Release/Debug flags (I add some debug/profiler switches there)
IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_C_FLAGS_DEBUG "-g -O3 ${PROFILER}")
	SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3 ${PROFILER}")
	SET(CMAKE_CXX_FLAGS_DISTRIBUTION "-O3")
	SET(CMAKE_C_FLAGS_DISTRIBUTION "-O3")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# MSVC specific stuff
IF(CMAKE_COMPILER_IS_MSVC)
	# STUB. Nothing now
ENDIF(CMAKE_COMPILER_IS_MSVC)

# Various modules used for library path detections.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Global debug option test. Sets the DEBUG flag in the config.h
IF(GLOBAL_DEBUG)
    MESSAGE("Debugging is ON")
    SET(OPDE_DEBUG 1)
ELSE(GLOBAL_DEBUG)
    MESSAGE("Debugging is Off")
ENDIF(GLOBAL_DEBUG)

# Some configuration checks to allow platform independence
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )

# Solve the dependencies
# SET(EXPAT_DIR ${CMAKE_MODULE_PATH})
SET(OGRE_DIR ${CMAKE_MODULE_PATH})
SET(CEGUI_DIR ${CMAKE_MODULE_PATH})

# FIND_PACKAGE(EXPAT)
FIND_PACKAGE(OGRE)
FIND_PACKAGE(CEGUI)

# Only visual check enabling verbose output, and package found info

# Expat not yet added. Needs a cross-platform compilation checks
# IF(EXPAT_FOUND_OK)
#     MESSAGE("EXPAT Found OK")
#     MESSAGE(" - EXPAT includes ${EXPAT_INCLUDE_DIR}")
#     MESSAGE(" - EXPAT libs ${EXPAT_LIBRARIES}")
# ELSE(EXPAT_FOUND_OK)
#     MESSAGE(FATAL_ERROR "EXPAT not found")
# ENDIF(EXPAT_FOUND_OK)

IF(OGRE_FOUND_OK)
    MESSAGE("OGRE Found OK")
    MESSAGE(" - Ogre includes ${OGRE_INCLUDE_DIR}")
    MESSAGE(" - Ogre libs ${OGRE_LIBRARIES}")
ELSE(OGRE_FOUND_OK)
    MESSAGE(FATAL_ERROR "OGRE not found")
ENDIF(OGRE_FOUND_OK)

IF(CEGUI_FOUND_OK)
    MESSAGE("CEGUI Found OK")
    MESSAGE(" - CEGUI includes ${CEGUI_INCLUDE_DIR}")
    MESSAGE(" - CEGUI libs ${CEGUI_LIBRARIES}")
ELSE(CEGUI_FOUND_OK)
    MESSAGE(FATAL_ERROR "CEGUI not found")
ENDIF(CEGUI_FOUND_OK)


# Add the subdirectories which contain aditional CMakeLists.txt files
add_subdirectory (src)
add_subdirectory (proto)

MESSAGE(STATUS "Your configuration seems to be OK!")

MESSAGE("")

# Print out some configuration status messages
MESSAGE("Your build parameters:")
MESSAGE(" * Global debugging : ${GLOBAL_DEBUG}")
MESSAGE(" * Build type       : ${CMAKE_BUILD_TYPE}")
MESSAGE(" * Profilling       : ${PROFILING}")
MESSAGE(" * C compiler       : ${CMAKE_C_COMPILER}")
MESSAGE(" * C++ compiler     : ${CMAKE_CXX_COMPILER}")
MESSAGE(" * Source directory : ${CMAKE_CURRENT_SOURCE_DIR}")
MESSAGE(" * Binary directory : ${CMAKE_CURRENT_BINARY_DIR}")
