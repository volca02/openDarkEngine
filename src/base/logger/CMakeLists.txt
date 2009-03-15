include_directories ( 
    ${OPDE_BINARY_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OPDE_SOURCE_DIR}/src/base
)

add_library(OpdeLogger 
    logger.cpp
    logger.h
    stdlog.cpp
    stdlog.h
    filelog.cpp
    filelog.h
    OgreOpdeLogConnector.cpp
    OgreOpdeLogConnector.h
)

SET_TARGET_PROPERTIES(OpdeLogger PROPERTIES
  COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
)
