INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
    ${OPDE_BASE_INCLUDES}
    ${PYTHON_INCLUDE_PATH}
    ${OPDE_SOURCE_DIR}/src/bindings
)

link_directories ( 
    ${OGRE_LIB_DIR}
)

add_library (OpdeScriptService ScriptService.cpp ScriptService.h)

target_link_libraries(OpdeScriptService
    OpdeBase
    OpdeServiceManager
)