# Dynamic types library

include_directories ( 
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/file
    ${OPDE_SOURCE_DIR}/src/compat
)

link_directories (
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
)

file(GLOB OPDE_DYNTYPE_HEADERS *.h)

# Various types of file access classes.
add_library (OpdeDynType 
    DVariant.cpp 
    DTypeDef.cpp
    ${OPDE_DYNTYPE_HEADERS}
)

target_link_libraries( OpdeDynType
	${OGRE_LIBRARIES}
	OpdeBase
)
