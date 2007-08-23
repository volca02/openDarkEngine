#
# Try to find OIS library on both Windows and Linux systems
# 
# The following values will get defined:
#	OIS_FOUND - True if ogre lib was found
#	OIS_INCLUDE_DIR - Include libraries for Ogre usage
#	OIS_LIBRARIES - Library paths for ogre

SET(OIS_FOUND)
MARK_AS_ADVANCED(OIS_FOUND)

IF (UNIX)
    INCLUDE(UsePkgConfig)

    PKGCONFIG(OIS OIS_INCLUDE_DIR OIS_LIBRARIES OIS_LIB_FLAGS OIS_CXX_FLAGS)

    IF (OIS_INCLUDE_DIR)
        # Move the -I include directories to the INCLUDE list
	STRING(REGEX MATCHALL "-I[^ ]+" OIS_INCLUDE_DIR "${OIS_CXX_FLAGS}")
        STRING(REGEX REPLACE "-I" "" OIS_INCLUDE_DIR "${OIS_INCLUDE_DIR}")

	# Process the linked libraries list from LIB_FLAGS to LIBRARIES
        STRING(REGEX MATCHALL "-l[^ ]+" OIS_LIBRARIES "${OIS_LIB_FLAGS}")
	STRING(REGEX REPLACE "-l" "" OIS_LIBRARIES "${OIS_LIBRARIES}")
    ENDIF(OIS_INCLUDE_DIR)
    
    IF (OIS_INCLUDE_DIR AND OIS_LIBRARIES)
	SET(OIS_FOUND "Yes")
    ELSE (OIS_INCLUDE_DIR AND OIS_LIBRARIES)
	MESSAGE(FATAL_ERROR "OIS was not found, but is required by this project")
    ENDIF (OIS_INCLUDE_DIR AND OIS_LIBRARIES)
ENDIF(UNIX)

IF(WIN32)
    # Include path when using the ogre SDK. Other drives/dirs?
    SET(OIS_INC_SEARCH_PATH
	/OIS/includes
	$ENV{OIS_HOME}/includes
     )
     
     SET(OIS_LIB_SEARCH_PATH
     	/OIS/lib
	/OIS/bin/debug
	/OIS/bin/release
	$ENV{OIS_HOME}/lib
	$ENV{OIS_HOME}/bin/debug
	$ENV{OIS_HOME}/bin/release
     )

	SET(OIS_LIBNAMES OIS)

	# Include paths search
	FIND_PATH(OIS_INCLUDE_DIR OIS.h ${OIS_INC_SEARCH_PATH})

	# OIS library
	FIND_LIBRARY(OIS_LIBRARIES NAMES ${OIS_LIBNAMES} PATHS 
		${OIS_LIB_SEARCH_PATH}
	)

	IF(OIS_INCLUDE_DIR AND OIS_LIBRARIES)
		SET(OIS_FOUND "Yes")
	ENDIF(OIS_INCLUDE_DIR AND OIS_LIBRARIES)		
ENDIF(WIN32)

