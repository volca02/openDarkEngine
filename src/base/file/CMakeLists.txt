include_directories ( 
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/compat
)

link_directories (
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
)

# Various types of file access classes.
add_library (OpdeFile File.cpp FileGroup.cpp)

target_link_libraries( OpdeFile
	OpdeBase
)
