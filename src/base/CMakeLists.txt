INCLUDE(${OPDE_SOURCE_DIR}/src/base/Base.cmake)

include_directories(
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/base/Dark
)

include_directories(
    ${OPDE_BINARY_DIR}
)

IF(STATIC_LIBS)
    add_library ( OpdeBase STATIC
        ${OPDE_BASE_FILES} # As in the Base.cmake
    )
ENDIF(STATIC_LIBS)
