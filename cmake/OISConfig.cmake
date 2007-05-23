#
# Try to find OIS library on both Windows and Linux systems
# 
# The following values will get defined:
#	OIS_FOUND_OK - True if ogre lib was found
#	OIS_INCLUDE_DIR - Include libraries for Ogre usage
#	OIS_LIBRARIES - Library paths for ogre

SET(OIS_FOUND_OK "No")

IF (UNIX)
    SET(OIS_INC_SEARCH_PATH
	/usr/include/OIS
	/usr/local/include/OIS
     )
     
     SET(OIS_LIB_SEARCH_PATH
        /lib
        /usr/lib
	/usr/lib32
	/usr/lib64
	/usr/local/lib
	/usr/local/lib32
	/usr/local/lib64
     )

	SET(OIS_LIBNAMES OIS)

	# Include paths search
	FIND_PATH(OIS_INCLUDE_DIR OIS.h ${OIS_INC_SEARCH_PATH})

	# Ogre library
	FIND_LIBRARY(OIS_LIBRARIES NAMES ${OIS_LIBNAMES} PATHS 
		${OIS_LIB_SEARCH_PATH}
	)


	MARK_AS_ADVANCED(OIS_FOUND_OK)

	IF(OIS_INCLUDE_DIR AND OIS_LIBRARIES)
	    SET(OIS_FOUND_OK "Yes")
	ENDIF(OIS_INCLUDE_DIR AND OIS_LIBRARIES)
ENDIF(UNIX)

IF(WIN32)
    # Include path when using the ogre SDK. Other drives/dirs?
    SET(OIS_INC_SEARCH_PATH
	/OgreSDK/include
	$ENV{OIS_HOME}/include
     )
     
     SET(OIS_LIB_SEARCH_PATH
     	c:/OIS/lib
	c:/OIS/bin/debug
	c:/OIS/bin/release
	$ENV{OIS_HOME}/lib
	$ENV{OIS_HOME}/bin/debug
	$ENV{OIS_HOME}/bin/release
     )

	SET(OIS_LIBNAMES OIS)

	# Include paths search
	FIND_PATH(OIS_INCLUDE_DIR OIS.h ${OIS_INC_SEARCH_PATH})

	# Ogre library
	FIND_LIBRARY(OIS_LIBRARIES NAMES ${OIS_LIBNAMES} PATHS 
		${OIS_LIB_SEARCH_PATH}
	)

	MARK_AS_ADVANCED(OIS_FOUND_OK)

	IF(OIS_INCLUDE_DIR AND OIS_LIBRARIES)
		SET(OIS_FOUND_OK "Yes")
	ENDIF(OIS_INCLUDE_DIR AND OIS_LIBRARIES)		
ENDIF(WIN32)

