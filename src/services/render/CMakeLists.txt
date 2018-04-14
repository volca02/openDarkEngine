INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)

include_directories (
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SERVICE_INCLUDES}
)

link_directories (
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

IF(STATIC_LIBS)
    add_library (OpdeRenderService
        RenderService.cpp
        RenderService.h
        MaterialInstance.cpp
        MaterialInstance.h
        SubEntityMaterialInstance.cpp
        SubEntityMaterialInstance.h
        EntityMaterialInstance.cpp
        EntityMaterialInstance.h
    )

    SET_TARGET_PROPERTIES(OpdeRenderService PROPERTIES
        COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
    )
ENDIF(STATIC_LIBS)
