#
# Try to find Cegui library on both Windows and Linux systems
# 
# The following values will get defined:
#	CEGUI_FOUND - True if cegui lib was found
#	CEGUI_INCLUDE_DIR - Include libraries for cegui usage
#	CEGUI_LIBRARIES - Library paths for cegui
#
# Please set your environmental variables to include CEGUI_HOME, which should point to the CEGUI SDK
#
SET(CEGUI_FOUND "No")

IF (UNIX)
    SET(CEGUI_INC_SEARCH_PATH
	/usr/include/cegui
	/usr/include/CEGUI
	/usr/local/include/cegui
	/usr/local/include/CEGUI
     )
     
     SET(CEGUI_LIB_SEARCH_PATH
        /lib
        /usr/lib
	/usr/lib32
	/usr/lib64
	/usr/local/lib
	/usr/local/lib32
	/usr/local/lib64
     )
 
ENDIF(UNIX)


IF(WIN32)
    SET(CEGUI_INC_SEARCH_PATH
	$ENV{CEGUI_HOME}/include
	$ENV{CEGUI_HOME}/include/CEGUI
	$ENV{CEGUI_EXTRA_INCLUDE_DIR}
	$ENV{CEGUI_EXTRA_INCLUDE_DIR}/include
     )
     
     SET(CEGUI_LIB_SEARCH_PATH
	$ENV{CEGUI_HOME}/lib
	$ENV{CEGUI_HOME}/bin
	$ENV{CEGUI_HOME}/bin/debug
	$ENV{CEGUI_HOME}/bin/release
     )

ENDIF(WIN32)

# Include paths search
FIND_PATH(CEGUI_INCLUDE_DIR CEGUI.h ${CEGUI_INC_SEARCH_PATH})

# Include paths search, renderer
FIND_PATH(CEGUI_EXTRA_INCLUDE_DIR OgreCEGUIRenderer.h ${CEGUI_INC_SEARCH_PATH})

# Cegui library
FIND_LIBRARY(CEGUI_CEGUI_LIB NAME CEGUIBase CEGUIBase_d PATHS ${CEGUI_LIB_SEARCH_PATH})

IF(NOT CEGUI_CEGUI_LIB)
	MESSAGE("WARNING: CEGUI library CEGUIBase not found under ${CEGUI_LIB_SEARCH_PATH}")
ENDIF(NOT CEGUI_CEGUI_LIB)

# Cegui ogre renderer library
FIND_LIBRARY(CEGUI_OGRE_LIB CEGUIOgreRenderer OgreGUIRenderer PATHS ${CEGUI_LIB_SEARCH_PATH})

IF(NOT CEGUI_OGRE_LIB)
	MESSAGE("WARNING: CEGUI OGRE library CEGUIOgreRenderer not found under ${CEGUI_LIB_SEARCH_PATH}")
ENDIF(NOT CEGUI_OGRE_LIB)


# Extra paths for include of CEGUI (for CEGUIOgreRenderer.h)
IF(CEGUI_EXTRA_INCLUDE_DIR)
    SET(CEGUI_INCLUDE_DIR ${CEGUI_EXTRA_INCLUDE_DIR} ${CEGUI_INCLUDE_DIR})
ENDIF(CEGUI_EXTRA_INCLUDE_DIR)

SET(CEGUI_LIBRARIES ${CEGUI_CEGUI_LIB} ${CEGUI_OGRE_LIB})

IF(CEGUI_INCLUDE_DIR AND CEGUI_CEGUI_LIB AND CEGUI_OGRE_LIB)
    SET(CEGUI_FOUND "Yes")
ENDIF(CEGUI_INCLUDE_DIR AND CEGUI_CEGUI_LIB AND CEGUI_OGRE_LIB)
