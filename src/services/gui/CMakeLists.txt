INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)
INCLUDE(${OPDE_SOURCE_DIR}/src/base/Base.cmake)

include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OIS_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}   
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SERVICE_INCLUDES}
    ${OPDE_SOURCE_DIR}/thirdparty/QuickGUI/include
)

link_directories ( 
    ${OGRE_LIB_DIR}
    ${OIS_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

add_library (OpdeGUIService 
    GUIService.cpp
    GUIService.h
)

target_link_libraries (OpdeGUIService
    OpdeServiceManager
    OpdeBase
)