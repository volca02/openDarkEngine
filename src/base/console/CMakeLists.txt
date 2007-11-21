include_directories ( 
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/logger
)

link_directories ( ${OGRE_LIB_DIR} )

file(GLOB OPDE_CONSOLE_HEADERS *.h)

add_library (OpdeConsole ConsoleBackend.cpp ConsoleFrontend.cpp ${OPDE_CONSOLE_HEADERS})
