include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_SOURCE_DIR}
    ${OPDE_SOURCE_DIR}/src/compat/
    ${OPDE_SOURCE_DIR}/src/base/servicemanager/
    ${OPDE_SOURCE_DIR}/src/base/console
    ${OPDE_SOURCE_DIR}/src/base/
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/services/database/
)

add_library (OpdeDatabaseService DarkDatabase.cpp)
