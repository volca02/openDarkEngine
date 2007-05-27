include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/base/dtype
    ${OPDE_SOURCE_DIR}/src/base/file
)

link_directories ( 
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

add_library (OpdeLinkService LinkService.cpp)

target_link_libraries(OpdeLinkService
    OpdeBase
    OpdeServiceManager
)