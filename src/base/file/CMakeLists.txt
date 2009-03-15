include_directories ( 
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/compat
)

link_directories (
    ${OGRE_LIB_DIR}
)

file(GLOB OPDE_FILE_HEADERS *.h)

# Various types of file access classes.
add_library (OpdeFile File.cpp FileGroup.cpp ${OPDE_FILE_HEADERS})

target_link_libraries( OpdeFile
	OpdeBase
)

SET_TARGET_PROPERTIES(OpdeFile PROPERTIES
  COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
)
