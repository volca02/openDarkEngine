include_directories ( 
    ${OPDE_BINARY_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OPDE_SOURCE_DIR}/src/base
)

file(GLOB OPDE_LOGGER_HEADERS *.h)

add_library(OpdeLogger 
    logger.cpp
    stdlog.cpp 
    filelog.cpp 
    OgreOpdeLogConnector.cpp
    ${OPDE_LOGGER_HEADERS}
)

SET_TARGET_PROPERTIES(OpdeLogger PROPERTIES
  COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
)
