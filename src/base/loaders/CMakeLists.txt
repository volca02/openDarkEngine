include_directories (
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${FREEIMAGE_INCLUDE_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/file
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/compat
)

link_directories (
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
)

file(GLOB OPDE_MANUALLOADERS_HEADERS *.h)

# Various types of manual resource loaders
add_library (OpdeManualLoaders ManualBinFileLoader.cpp ManualFonFileLoader.cpp ${OPDE_MANUALLOADERS_HEADERS})

target_link_libraries( OpdeManualLoaders
	OpdeBase
)
