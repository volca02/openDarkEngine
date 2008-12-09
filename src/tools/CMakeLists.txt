include_directories (
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/file
)

add_executable ( meshconvert meshconvert.cpp )

add_executable ( chunk chunk.cpp )

add_executable ( DarkFontConverter DarkFontConverter.cpp )

# TODO: Only compile-in the virtual file system statically instead of the whole OpenDark library 
target_link_libraries( chunk
        ${OGRE_LIBRARIES}
        OpenDark
)
