include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/base/dyntype
    ${OPDE_SOURCE_DIR}/src/base/file
    ${OPDE_SOURCE_DIR}/src/base/console
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OPDE_SOURCE_DIR}/src/services
    ${OPDE_SOURCE_DIR}/src/services/binary
    ${OPDE_SOURCE_DIR}/src/services/link
    ${OPDE_SOURCE_DIR}/src/services/inherit
    ${OPDE_SOURCE_DIR}/src/services/database
)

link_directories ( 
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

add_library (OpdePropertyService PropertyService.cpp PropertyGroup.cpp)

target_link_libraries(OpdePropertyService
    OpdeBase
    OpdeServiceManager
)