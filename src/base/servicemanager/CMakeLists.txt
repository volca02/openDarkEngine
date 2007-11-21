include_directories ( 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/ 
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OGRE_INCLUDE_DIR}
)

file(GLOB OPDE_SERVICEMANAGER_HEADERS *.h)

add_library (OpdeServiceManager OpdeServiceManager.cpp OpdeService.cpp ${OPDE_SERVICEMANAGER_HEADERS})
