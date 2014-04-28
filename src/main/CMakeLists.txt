# $Id$

SET(OPDE_SERVICE_FILES)
SET(OPDE_BASE_FILES)
SET(OPDE_BINDING_FILES)
SET(OPDE_SCENEMANAGER_FILES)

INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/bindings/Bindings.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/scenemanager/DarkSceneManager.cmake )

include_directories(
    ${ODE_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/bindings
    ${OPDE_SERVICE_INCLUDES}
    ${FREEIMAGE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_PATH}
)

IF(SCRIPT_COMPILERS)
    SET(OPDE_SCRIPT_COMPILER_FILES
        DTypeScriptCompiler.cpp
        DTypeScriptCompiler.h
        PLDefScriptCompiler.cpp
        PLDefScriptCompiler.h
        DTypeScriptLoader.cpp
        DTypeScriptLoader.h
        PLDefScriptLoader.cpp
        PLDefScriptLoader.h
    )
ELSE(SCRIPT_COMPILERS)
    SET(OPDE_SCRIPT_COMPILER_FILES)
ENDIF(SCRIPT_COMPILERS)


add_library (OpenDark SHARED
    Root.cpp
    Root.h
    ${OPDE_SCRIPT_COMPILER_FILES}
    CustomImageCodec.cpp
    CustomImageCodec.h
    ProxyArchive.cpp
    ProxyArchive.h
    OgreZip.cpp
    OgreZip.h
    # and the base lib files
    ${OPDE_BASE_FILES}
    # and service files as well
    ${OPDE_SERVICE_FILES}
    # and the python bindings
    ${OPDE_BINDING_FILES}
    # and the scene manager
    ${OPDE_SCENEMANAGER_FILES}
)

IF (PYTHON_MODULE)
    # Library that exposes the OpenDark lib as a python module - for direct python usage
    PYTHON_ADD_MODULE(opde
        PyRoot.cpp
    )

    target_link_libraries(opde
        OpenDark
        ${OGRE_LIBRARIES}
        ${ODE_LIBRARIES}
        ${SDL2_LIBRARIES}
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

# We don't link any our own files here, as CMake does not support convenience libraries
target_link_libraries( OpenDark
    LINK_PRIVATE
    ${OGRE_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${ODE_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    ${OPDE_PYTHON_LIBRARIES}
)

set_property(TARGET OpenDark PROPERTY LINK_INTERFACE_LIBRARIES "")

# Build flags for the .DLL __declspec things, etc.
SET_TARGET_PROPERTIES(OpenDark PROPERTIES
  DEFINE_SYMBOL "OPDELIB_DLL_TARGET"
  LINK_INTERFACE_LIBRARIES ""
)

if (UNIX)
    # Currently there is no need to build this on windows (and also it fails) - is there latex2html for windows anyway?
    # Doc generator for opde - property and link types info generator
    add_executable (opdeDocGen
        OpdeDocGen.cpp
    )

    target_link_libraries( opdeDocGen
        OpenDark
        ${OGRE_LIBRARIES}
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
)

target_link_libraries( opdeMain
    OpenDark
    ${OGRE_LIBRARIES}
    ${ODE_LIBRARIES}
    ${SDL2_LIBRARIES}
)

SET_TARGET_PROPERTIES( opdeMain PROPERTIES
  COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
)

install(TARGETS opdeMain OpenDark
	RUNTIME DESTINATION ${OPDE_BINARY_INSTALL_DIR}
	LIBRARY DESTINATION ${OPDE_LIBRARY_INSTALL_DIR}
)

# Script executor executable
add_executable (opdeScript
    OpdeScript.cpp
)

target_link_libraries( opdeScript
    OpenDark
    ${OGRE_LIBRARIES}
    ${ODE_LIBRARIES}
    ${OPDE_PYTHON_LIBRARIES}
    ${SDL2_LIBRARIES}
)

SET_TARGET_PROPERTIES( opdeScript PROPERTIES
  COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
)
