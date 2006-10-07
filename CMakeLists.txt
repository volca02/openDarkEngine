project (OPDE)

# MinGW specific
IF(CMAKE_COMPILER_IS_MINGW)
	SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
	SET(CMAKE_SHARED_LINKER_FLAGS"-Wl,--enable-auto-image-base -Wl,--add-stdcall-alias -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import")
ENDIF(CMAKE_COMPILER_IS_MINGW)


# Various modules used for library path detections.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Some configuration checks to allow platform independence
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h )

# Solve the dependencies
SET(OGRE_DIR ${CMAKE_MODULE_PATH})
SET(CEGUI_DIR ${CMAKE_MODULE_PATH})

FIND_PACKAGE(OGRE)
FIND_PACKAGE(CEGUI)

IF(OGRE_FOUND)
    MESSAGE("OGRE Found OK")
    MESSAGE(" - Ogre includes ${OGRE_INCLUDE_DIR}")
    MESSAGE(" - Ogre libs ${OGRE_LIBRARIES}")
ELSE(OGRE_FOUND)
    MESSAGE(FATAL_ERROR "OGRE not found")
ENDIF(OGRE_FOUND)

IF(CEGUI_FOUND)
    MESSAGE("CEGUI Found OK")
    MESSAGE(" - CEGUI includes ${CEGUI_INCLUDE_DIR}")
    MESSAGE(" - CEGUI libs ${CEGUI_LIBRARIES}")
ELSE(CEGUI_FOUND)
    MESSAGE(FATAL_ERROR "CEGUI not found")
ENDIF(CEGUI_FOUND)

add_subdirectory (src)
add_subdirectory (proto)
