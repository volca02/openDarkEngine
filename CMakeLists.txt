project (OPDE)

# Build type selection. This should be targettable 
SET(CMAKE_BUILD_TYPE Debug)

# Option to build in the debugging code
OPTION(GLOBAL_DEBUG 
    "Build the debugging code too."
    OFF)

#Target the binary files into a separate directories
# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR/Build})

# MinGW specific
IF(CMAKE_COMPILER_IS_MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
	SET(CMAKE_SHARED_LINKER_FLAGS"-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
ENDIF(CMAKE_COMPILER_IS_MINGW)

# Warning level high
SET(CMAKE_CXX_WARNING_LEVEL 4)

# GCC specific. Release/Debug flags (I add some debug/profiler switches there)
IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_C_FLAGS_DEBUG "-g -O3 -pg")
	SET(CMAKE_CXX_FLAGS_DEBUG "-g -O3 -pg")
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


add_subdirectory (src)
add_subdirectory (proto)
