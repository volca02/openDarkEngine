include_directories ( 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/ 
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OGRE_INCLUDE_DIR}
)

add_library ( OpdeServiceManager
    OpdeServiceManager.cpp
    OpdeServiceManager.h
    OpdeService.cpp
    OpdeService.h
    OpdeServiceFactory.h
)

SET_TARGET_PROPERTIES(OpdeServiceManager PROPERTIES
  COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
)
