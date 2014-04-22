#
# Try to find Ogre library on both Windows and Linux systems
#
# The following values will get defined:
#	OGRE_FOUND - True if ogre lib was found
#	OGRE_INCLUDE_DIR - Include libraries for Ogre usage
#	OGRE_LIBRARIES - Library paths for ogre

SET(OGRE_FOUND "No")
MARK_AS_ADVANCED(OGRE_FOUND)

IF (UNIX)
    # Let's use pkg-config!

    INCLUDE(UsePkgConfig)

    PKGCONFIG(OGRE OGRE_INCLUDE_DIR OGRE_MAIN_LIBRARIES OGRE_LIB_FLAGS OGRE_CXX_FLAGS)
    PKGCONFIG(OGRE-Overlay OGRE_INCLUDE_DIR OGRE_OVERLAY_LIBRARIES OGRE_LIB_FLAGS OGRE_CXX_FLAGS)

    SET(OGRE_LIBRARIES ${OGRE_MAIN_LIBRARIES} ${OGRE_OVERLAY_LIBRARIES})

    IF (OGRE_INCLUDE_DIR)
        # Move the -I include directories to the INCLUDE list
        STRING(REGEX MATCHALL "-I[^ ]+" OGRE_INCLUDE_DIR "${OGRE_CXX_FLAGS}")
        STRING(REGEX REPLACE "-I" "" OGRE_INCLUDE_DIR "${OGRE_INCLUDE_DIR}")

        # Process the linked libraries list from LIB_FLAGS to LIBRARIES
        STRING(REGEX MATCHALL "-l[^ ]+" OGRE_LIBRARIES "${OGRE_LIB_FLAGS}")
        STRING(REGEX REPLACE "-l" "" OGRE_LIBRARIES "${OGRE_LIBRARIES}")
    ENDIF(OGRE_INCLUDE_DIR)

    IF (OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
        SET(OGRE_FOUND "Yes")
    ELSE (OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
        MESSAGE(FATAL_ERROR "OGRE was not found, but is required by this project")
    ENDIF (OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
ENDIF(UNIX)

IF(WIN32)
    # Include path when using the ogre SDK. Other drives/dirs?
    SET(OGRE_INC_SEARCH_PATH
      /OgreSDK/include
      $ENV{OGRE_HOME}/include
     )

     SET(OGRE_LIB_SEARCH_PATH
       c:/OgreSDK/lib
       c:/OgreSDK/bin/debug
       c:/OgreSDK/bin/release
       $ENV{OGRE_HOME}/lib
       $ENV{OGRE_HOME}/bin/debug
       $ENV{OGRE_HOME}/bin/release
     )

    IF(MSVC)
        # TODO: Find ogre overlay. Or don't, we're gonna get rid of it.
        SET(OGRE_LIBNAMES_DEB OgreMain_d)
        SET(OGRE_LIBNAMES_REL OgreMain)

        # Include paths search
        FIND_PATH(OGRE_INCLUDE_DIR Ogre.h ${OGRE_INC_SEARCH_PATH})

        # Ogre library
        FIND_LIBRARY(OGRE_LIBRARIES_DEBUG NAMES ${OGRE_LIBNAMES_DEB} PATHS
            ${OGRE_LIB_SEARCH_PATH}
        )

        FIND_LIBRARY(OGRE_LIBRARIES_RELEASE NAMES ${OGRE_LIBNAMES_REL} PATHS
            ${OGRE_LIB_SEARCH_PATH}
        )

        IF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES_RELEASE AND OGRE_LIBRARIES_DEBUG)
            SET(OGRE_LIBRARIES optimized ${OGRE_LIBRARIES_RELEASE} debug ${OGRE_LIBRARIES_DEBUG})
        ENDIF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES_RELEASE AND OGRE_LIBRARIES_DEBUG)
    ELSE(MSVC)
        SET(OGRE_LIBNAMES OgreMain)
        SET(OGRE_OVERLAY_LIBNAMES OgreOverlay)

        # Include paths search
        FIND_PATH(OGRE_INCLUDE_DIR Ogre.h ${OGRE_INC_SEARCH_PATH})

        # Ogre library
        FIND_LIBRARY(OGRE_MAIN_LIB NAMES ${OGRE_LIBNAMES} PATHS
            ${OGRE_LIB_SEARCH_PATH}
        )

        FIND_LIBRARY(OGRE_MOD_OVERLAY NAMES ${OGRE_OVERLAY_LIBNAMES} PATHS
            ${OGRE_LIB_SEARCH_PATH}
        )

        SET(OGRE_LIBRARIES ${OGRE_MAIN_LIB} ${OGRE_MOD_OVERLAY})
    ENDIF(MSVC)
ENDIF(WIN32)

IF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
    SET(OGRE_FOUND "Yes")
ENDIF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
