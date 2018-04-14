include_directories (
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
)

link_directories (
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/dyntype
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

add_library (OpdeBinaryService OBJECT BinaryService.cpp BinaryService.h)
