INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

include_directories(
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/widgets
    ${OPDE_SERVICE_INCLUDES}
    ${FREEIMAGE_INCLUDE_DIR}
)

add_library (OpdeLib SHARED
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
)

set_property(TARGET OpdeLib PROPERTY LINK_INTERFACE_LIBRARIES "")

target_link_libraries( OpdeLib
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    ${OPDE_BASE_LIBRARIES}
    DarkSceneManager
    ${OPDE_SERVICE_LIBRARIES}
    QuickGUI
)

# Build flags for the .DLL __declspec things, etc.
SET_TARGET_PROPERTIES(OpdeLib PROPERTIES
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
        OpdeLib
    )

    SET_TARGET_PROPERTIES(opdeDocGen PROPERTIES
      DEFINE_SYMBOL "OPDE_EXE_TARGET"
    )
endif(NOT WIN32)

# Main executable
add_executable (opdemain WIN32 
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

target_link_libraries( opdemain
    OpdeLib
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
)

SET_TARGET_PROPERTIES( opdemain PROPERTIES
  COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
)
