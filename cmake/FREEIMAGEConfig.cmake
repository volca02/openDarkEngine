#
# Try to find FREEIMAGE library on both Windows and Linux systems
# 
# The following values will get defined:
#	FREEIMAGE_FOUND - True if FreeImage lib was found
#	FREEIMAGE_INCLUDE_DIR - Include libraries for FreeImage usage
#	FREEIMAGE_LIBRARIES - Library paths for FreeImage
#
# $Id$

SET(FREEIMAGE_FOUND)
MARK_AS_ADVANCED(FREEIMAGE_FOUND)
MARK_AS_ADVANCED(FREEIMAGE_LIBRARIES)
MARK_AS_ADVANCED(FREEIMAGE_INCLUDE_DIR)

# For win32 builds. This can be filled by hand. Standard path is dependencies directory next to opde binary dir (out of source build target directory)
SET(FREEIMAGE_PATH "${OPDE_BINARY_DIR}/../dependencies/freeimage/" CACHE STRING "For Win32 lib. search. Can be filled to the path of the unzipped FreeImageXXXXWin32.zip (Including FreeImage Directory)")

IF (UNIX)
    # Hide the FREEIMAGE_PATH, not needed under unix systems
    MARK_AS_ADVANCED(FREEIMAGE_PATH)
    
    # Let's look for lib freeimage
    # There is no pkg-config for freeimage, so we have to search by hand
    SET(FREEIMAGE_INC_SEARCH_PATH
        /usr/include/
        /usr/local/include/
     )

     SET(FREEIMAGE_LIB_SEARCH_PATH
        /lib
        /usr/lib
        /usr/lib32
        /usr/lib64
        /usr/local/lib
        /usr/local/lib32
        /usr/local/lib64
     )

    # That's it. The library name without the lib and .so
    SET(FREEIMAGE_LIBNAMES freeimage)
    
    # The search begins
    FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h ${FREEIMAGE_INC_SEARCH_PATH})

    IF(NOT FREEIMAGE_INCLUDE_DIR)
        MESSAGE("Warning: FreeImage.h not found under ${FREEIMAGE_INC_SEARCH_PATH}")
        SET(FREEIMAGE_FOUND 0)
    ENDIF(NOT FREEIMAGE_INCLUDE_DIR)
    
    # Headers up ok, now the lib
    FIND_LIBRARY(FREEIMAGE_LIBRARIES NAME ${FREEIMAGE_LIBNAMES} PATHS ${FREEIMAGE_SEARCH_PATH})

    IF(NOT FREEIMAGE_LIBRARIES)
        MESSAGE("Warning: FreeImage library not found under ${FREEIMAGE_LIB_SEARCH_PATH}")
        SET(FREEIMAGE_FOUND 0)
    ENDIF(NOT FREEIMAGE_LIBRARIES)
ENDIF(UNIX)

IF(WIN32)
    SET(FREEIMAGE_INC_SEARCH_PATH
        $ENV{FREEIMAGE_PATH}/Dist
	${FREEIMAGE_PATH}/Dist
    )

    SET(FREEIMAGE_LIB_SEARCH_PATH
        $ENV{FREEIMAGE_PATH}/Dist
	${FREEIMAGE_PATH}/Dist
    )

    # That's it. The library name without the lib and .so
    SET(FREEIMAGE_LIBNAMES freeimage)
    
    # The search begins
    FIND_PATH(FREEIMAGE_INCLUDE_DIR FreeImage.h ${FREEIMAGE_INC_SEARCH_PATH})

    IF(NOT FREEIMAGE_INCLUDE_DIR)
        MESSAGE("Warning: FreeImage.h not found under ${FREEIMAGE_INC_SEARCH_PATH}")
        SET(FREEIMAGE_FOUND 0)
    ENDIF(NOT FREEIMAGE_INCLUDE_DIR)
    
    # Headers up ok, now the lib
    FIND_LIBRARY(FREEIMAGE_LIBRARIES NAME ${FREEIMAGE_LIBNAMES} PATHS ${FREEIMAGE_LIB_SEARCH_PATH})

    IF(NOT FREEIMAGE_LIBRARIES)
        MESSAGE("Warning: FreeImage library not found under ${FREEIMAGE_LIB_SEARCH_PATH}")
        SET(FREEIMAGE_FOUND 0)
    ENDIF(NOT FREEIMAGE_LIBRARIES)
ENDIF(WIN32)
