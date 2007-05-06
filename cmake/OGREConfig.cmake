#
# Try to find Ogre library on both Windows and Linux systems
# 
# The following values will get defined:
#	OGRE_FOUND_OK - True if ogre lib was found
#	OGRE_INCLUDE_DIR - Include libraries for Ogre usage
#	OGRE_LIBRARIES - Library paths for ogre

SET(OGRE_FOUND_OK "No")

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

	SET(OGRE_LIBNAMES OgreMain)

	# Include paths search
	FIND_PATH(OGRE_INCLUDE_DIR Ogre.h ${OGRE_INC_SEARCH_PATH})

	# Ogre library
	FIND_LIBRARY(OGRE_LIBRARIES NAMES ${OGRE_LIBNAMES} PATHS 
		${OGRE_LIB_SEARCH_PATH}
	)


	MARK_AS_ADVANCED(OGRE_FOUND_OK)

	IF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
	    SET(OGRE_FOUND_OK "Yes")
	ENDIF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
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

		MARK_AS_ADVANCED(OGRE_FOUND_OK)
	
		IF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES_RELEASE AND OGRE_LIBRARIES_DEBUG)
		    SET(OGRE_FOUND_OK "Yes")
			SET(OGRE_LIBRARIES optimized ${OGRE_LIBRARIES_RELEASE} debug ${OGRE_LIBRARIES_DEBUG})
		ENDIF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES_RELEASE AND OGRE_LIBRARIES_DEBUG)
	ELSE(MSVC)
		SET(OGRE_LIBNAMES OgreMain)

		# Include paths search
		FIND_PATH(OGRE_INCLUDE_DIR Ogre.h ${OGRE_INC_SEARCH_PATH})

		# Ogre library
		FIND_LIBRARY(OGRE_LIBRARIES NAMES ${OGRE_LIBNAMES} PATHS 
			${OGRE_LIB_SEARCH_PATH}
		)


		MARK_AS_ADVANCED(OGRE_FOUND_OK)

		IF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)
			SET(OGRE_FOUND_OK "Yes")
		ENDIF(OGRE_INCLUDE_DIR AND OGRE_LIBRARIES)		
	ENDIF(MSVC)
ENDIF(WIN32)
