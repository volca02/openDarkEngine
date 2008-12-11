INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/bindings/Bindings.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/scenemanager/DarkSceneManager.cmake )

include_directories(
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/bindings
    ${OPDE_SOURCE_DIR}/thirdparty/QuickGUI/include
    ${OPDE_SERVICE_INCLUDES}
    ${FREEIMAGE_INCLUDE_DIR}
    ${PYTHON_INCLUDE_PATH}
)

add_library (OpenDark SHARED
    Root.cpp
    Root.h
    DTypeScriptCompiler.cpp
    DTypeScriptCompiler.h
    PLDefScriptCompiler.cpp
    PLDefScriptCompiler.h
    DTypeScriptLoader.cpp
    DTypeScriptLoader.h
    PLDefScriptLoader.cpp
    PLDefScriptLoader.h
    CustomImageCodec.cpp
    CustomImageCodec.h
    ProxyArchive.cpp
    ProxyArchive.h
    # and the base lib files
    ${OPDE_BASE_FILES}
    # and service files as well
    ${OPDE_SERVICE_FILES}
    # and the python bindings
    ${OPDE_BINDING_FILES}
    # and the scene manager
    ${OPDE_SCENEMANAGER_FILES}
)

link_directories(
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/scenemanager
)

# We don't link any our own files here, as CMake does not support convenience libraries
target_link_libraries( OpenDark
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

set_property(TARGET OpenDark PROPERTY LINK_INTERFACE_LIBRARIES "")

# Build flags for the .DLL __declspec things, etc.
SET_TARGET_PROPERTIES(OpenDark PROPERTIES
  DEFINE_SYMBOL "OPDELIB_DLL_TARGET"
  LINK_INTERFACE_LIBRARIES ""
)

if (NOT WIN32)
    # Currently there is no need to build this on windows (and also it fails) - is there latex2html for windows anyway?
    # Doc generator for opde - property and link types info generator
    add_executable (opdeDocGen WIN32 
        OpdeDocGen.cpp
    )

    target_link_libraries( opdeDocGen
        OpenDark
    )

    SET_TARGET_PROPERTIES(opdeDocGen PROPERTIES
      COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
    )
endif(NOT WIN32)

# Main executable
add_executable (opde WIN32 
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

target_link_libraries( opde
    OpenDark
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
)

SET_TARGET_PROPERTIES( opde PROPERTIES
  COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
)

# Script executor executable
add_executable (opdeScript
    OpdeScript.cpp
)

target_link_libraries( opdeScript
    OpenDark
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES( opdeScript PROPERTIES
  COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
)

