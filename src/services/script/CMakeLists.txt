# $Id$

INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

include_directories ( 
    ${OIS_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
    ${OPDE_BASE_INCLUDES}
    ${PYTHON_INCLUDE_PATH}
    ${OPDE_SOURCE_DIR}/src/bindings
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/scenemanager
)

link_directories ( 
    ${OGRE_LIB_DIR}
)

IF(STATIC_LIBS)
    add_library (OpdeScriptService ScriptService.cpp ScriptService.h)

    SET_TARGET_PROPERTIES(OpdeScriptService PROPERTIES
      COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
    )
ENDIF(STATIC_LIBS)

