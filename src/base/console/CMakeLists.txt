include_directories ( 
    ${OGRE_INCLUDE_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/logger
)

link_directories ( ${OGRE_LIB_DIR} )

add_library (OpdeConsole ConsoleBackend.cpp)
