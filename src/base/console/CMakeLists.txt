# $Id$

include_directories ( 
	${ODE_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/logger
)

link_directories ( ${OGRE_LIB_DIR} )

add_library (OpdeConsole STATIC
    ConsoleBackend.cpp
    ConsoleBackend.h
    ConsoleFrontend.cpp
    ConsoleFrontend.h
    ConsoleCommandListener.cpp
    ConsoleCommandListener.h
)

SET_TARGET_PROPERTIES(OpdeConsole PROPERTIES
  COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
)
