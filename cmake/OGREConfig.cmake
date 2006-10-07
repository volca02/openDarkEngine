#
# Try to find Ogre library on both Windows and Linux systems
# 
# The following values will get defined:
#	OGRE_FOUND - True if ogre lib was found
#	OGRE_INCLUDE_DIR - Include libraries for Ogre usage
#	OGRE_LIBRARIES - Library paths for ogre

SET(OGRE_FOUND "No")

IF (UNIX)
    SET(OGRE_INC_SEARCH_PATH
	/usr/include/Ogre
	/usr/include/OGRE
	/usr/local/include/Ogre
	/usr/local/include/OGRE
     )
     
     SET(OGRE_LIB_SEARCH_PATH
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

ENDIF(WIN32)

# Include paths search
FIND_PATH(OGRE_INCLUDE_DIR Ogre.h ${OGRE_INC_SEARCH_PATH})

# Ogre library
FIND_LIBRARY(OGRE_LIBRARIES NAMES OgreMain OgreMain_d PATHS 
	${OGRE_LIB_SEARCH_PATH}
)



IF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
    SET(OGRE_FOUND "Yes")
ENDIF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)