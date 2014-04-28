INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)
INCLUDE(${OPDE_SOURCE_DIR}/src/base/Base.cmake)

include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}   
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SERVICE_INCLUDES}
)

link_directories ( 
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

IF(STATIC_LIBS)
    add_library (OpdeLoopService LoopService.cpp LoopService.h)

    SET_TARGET_PROPERTIES(OpdeLoopService PROPERTIES
	COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
    )
ENDIF(STATIC_LIBS)
