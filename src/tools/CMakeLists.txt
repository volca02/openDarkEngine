include_directories (
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/file
)

add_executable ( meshconvert meshconvert.cpp $<TARGET_OBJECTS:OpenDark>)

add_executable ( chunk chunk.cpp $<TARGET_OBJECTS:OpenDark>)

add_executable ( physver physver.cpp $<TARGET_OBJECTS:OpenDark>)

add_executable ( DarkFontConverter DarkFontConverter.cpp $<TARGET_OBJECTS:OpenDark>)

# TODO: Only compile-in the virtual file system statically instead of the whole OpenDark library
target_link_libraries( chunk
    ${OGRE_LIBRARIES}
    ${ODE_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
)

target_link_libraries( DarkFontConverter
    ${OGRE_LIBRARIES}
    ${ODE_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
)

target_link_libraries( physver
    ${OGRE_LIBRARIES}
    ${ODE_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
)
