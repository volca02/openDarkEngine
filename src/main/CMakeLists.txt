# $Id$

SET(OPDE_SERVICE_FILES)
SET(OPDE_BASE_FILES)
SET(OPDE_BINDING_FILES)
SET(OPDE_SCENEMANAGER_FILES)

include_directories(
    ${ODE_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/console
    ${OPDE_SOURCE_DIR}/src/base/compat
    ${OPDE_SOURCE_DIR}/src/base/dyntype
    ${OPDE_SOURCE_DIR}/src/base/file
    ${OPDE_SOURCE_DIR}/src/base/loaders
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/bindings
    ${OPDE_SOURCE_DIR}/src/services
    ${OPDE_SERVICE_INCLUDES}
    ${FREEIMAGE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_PATH}
)

# TODO: Split this into parts. We don't need the whole package for small utils
add_library (OpdeRoot OBJECT
    Root.cpp
    Root.h
    CustomImageCodec.cpp
    CustomImageCodec.h
    ProxyArchive.cpp
    ProxyArchive.h
# This is here because of this change:  dc38d2dcf502b6a3c21202573c6f2c3320a0687a
# that broken fully qualified file names in zip archives. It's fixed in recent
# ogre ONLY if OGRE_RESOURCEMANAGER_STRICT is defined
# HINT: not the case in aur package on arch
    OgreFixedZip.cpp
    OgreFixedZip.h
)

SET(OPDE_LIB_OBJECTS
  $<TARGET_OBJECTS:OpdeBase>
  $<TARGET_OBJECTS:OpdeServices>
  $<TARGET_OBJECTS:DarkSceneManager>
  $<TARGET_OBJECTS:OpdePython>
  $<TARGET_OBJECTS:OpdeRoot>
)

IF (PYTHON_MODULE)
    # Library that exposes the OpenDark lib as a python module - for direct python usage
    PYTHON_ADD_MODULE(opde
        PyRoot.cpp
        ${OPDE_LIB_OBJECTS}
        $<TARGET_OBJECTS:OpdePython>
    )

    target_link_libraries(opde
        ${OGRE_LIBRARIES}
        ${ODE_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${FREEIMAGE_LIBRARIES}
        ${OPDE_PYTHON_LIBRARIES}
    )

    # We need the library to be named *exactly* opde.so/opde.dll in order to find it with python
    # So we remove any platform specific library name prefix
    set_property(TARGET opde PROPERTY PREFIX "")

    # We install into the python's site-packages dir based on distutils
    # Thanks to PLPlot for this snippet
    execute_process(
        COMMAND
        ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print sysconfig.get_python_lib(1,0,prefix='${CMAKE_INSTALL_EXEC_PREFIX}')"
        OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Directory name for the module's headers
    if(USE_RPATH)
        set_target_properties(
            opde
            PROPERTIES
            INSTALL_RPATH "${LIB_INSTALL_RPATH}"
            INSTALL_NAME_DIR "${PYTHON_SITE_PACKAGES_DIR}"
        )
    else(USE_RPATH)
        set_target_properties(
            opde
            PROPERTIES
            INSTALL_NAME_DIR "${PYTHON_SITE_PACKAGES_DIR}"
        )
    endif(USE_RPATH)

    # install directives
    install(
        TARGETS opde
        LIBRARY
        DESTINATION ${PYTHON_SITE_PACKAGES_DIR}
    )
ENDIF(PYTHON_MODULE)

link_directories(
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/scenemanager
)

if (UNIX)
    # Currently there is no need to build this on windows (and also it fails) - is there latex2html for windows anyway?
    # Doc generator for opde - property and link types info generator
    add_executable (opdeDocGen
        OpdeDocGen.cpp
        ${OPDE_LIB_OBJECTS}
    )

    target_link_libraries( opdeDocGen
        ${OGRE_LIBRARIES}
        ${OGRE_LIBRARIES}
        ${ODE_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${FREEIMAGE_LIBRARIES}
        ${PYTHON_LIBRARIES}
        # TODO: REMOVE, TEMPORARY TILL WE CHANGE TO OGRE's STRICT RESOURCE MANAGER
        ${ZZIPLIB_LIBRARIES}
    )

    SET_TARGET_PROPERTIES(opdeDocGen PROPERTIES
      COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
    )
endif(UNIX)

# Main executable
add_executable (opdeMain WIN32
    Opde.cpp
    GameStateManager.cpp
    GameState.cpp
    GamePlayState.cpp
    GameLoadState.cpp
    GameStateManager.h
    GameState.h
    GamePlayState.h
    GameLoadState.h
    ${OPDE_LIB_OBJECTS}
)

target_link_libraries(opdeMain
    ${OGRE_LIBRARIES}
    ${ODE_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    ${PYTHON_LIBRARIES}
    # TODO: REMOVE, TEMPORARY TILL WE CHANGE TO OGRE's STRICT RESOURCE MANAGER
    ${ZZIPLIB_LIBRARIES}
)

install(TARGETS opdeMain
    RUNTIME DESTINATION ${OPDE_BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${OPDE_LIBRARY_INSTALL_DIR}
)

# Script executor executable
add_executable (opdeScript
    OpdeScript.cpp
    ${OPDE_LIB_OBJECTS}
)

target_link_libraries(opdeScript
    ${OGRE_LIBRARIES}
    ${ODE_LIBRARIES}
    ${OPDE_PYTHON_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    # TODO: REMOVE, TEMPORARY TILL WE CHANGE TO OGRE's STRICT RESOURCE MANAGER
    ${ZZIPLIB_LIBRARIES}
)
