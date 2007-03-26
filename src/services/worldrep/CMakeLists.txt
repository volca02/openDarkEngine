include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OPDE_SOURCE_DIR}/src/compat/
    ${OPDE_SOURCE_DIR}/src/base/servicemanager/
    ${OPDE_SOURCE_DIR}/src/base/console
    ${OPDE_SOURCE_DIR}/src/base/
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/scenemanager/
    ${OPDE_SOURCE_DIR}/src/services/database/
)

link_directories ( 
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base/servicemanager/
)

add_library (OpdeWorldRepService worldrep.cpp WRCell.cpp LightmapAtlas.cpp)

target_link_libraries(OpdeWorldRepService
    OpdeServiceManager
)