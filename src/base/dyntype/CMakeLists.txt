# Dynamic types library

include_directories ( 
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/file
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OPDE_SOURCE_DIR}/src/compat
)

link_directories (
    ${OGRE_LIB_DIR}
)

# Various types of file access classes.
add_library (OpdeDynType 
    DVariant.cpp 
    DVariant.h
    DTypeDef.cpp
    DTypeDef.h
    DTHelpers.h
    DataStorage.cpp
    DataStorage.h
    SingleFieldDataStorage.h
)

target_link_libraries( OpdeDynType
	${OGRE_LIBRARIES}
	OpdeBase
)

SET_TARGET_PROPERTIES(OpdeDynType PROPERTIES
  COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
)
