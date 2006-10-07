include_directories ( ${OGRE_INCLUDE_DIR} )
link_directories ( ${OGRE_LIB_DIR} )

add_library (OpdeConsole ConsoleBackend.cpp)
