INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)
INCLUDE(${OPDE_SOURCE_DIR}/src/base/Base.cmake)

include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OIS_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}   
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SERVICE_INCLUDES}
)

IF(STATIC_LIBS)
    add_library (OpdeGUIService 
	GUIService.cpp
        GUIService.h
    )

    SET_TARGET_PROPERTIES(OpdeGUIService PROPERTIES
	COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
    )
ENDIF(STATIC_LIBS)