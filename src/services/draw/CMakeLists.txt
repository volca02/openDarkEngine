include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/dyntype
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/services
)

link_directories ( 
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

IF(STATIC_LIBS)
    add_library (OpdeDrawService DrawService.cpp DrawService.h)

    SET_TARGET_PROPERTIES(OpdeDrawService PROPERTIES
	COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
    )
ENDIF(STATIC_LIBS)
