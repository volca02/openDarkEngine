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


link_directories(
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OPDE_SOURCE_DIR}/src/base/gui
    ${OPDE_SOURCE_DIR}/src/base/console
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/widgets
    ${OPDE_SOURCE_DIR}/src/services/database
    ${OPDE_SOURCE_DIR}/src/services/worldrep
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

target_link_libraries( OpdeLib
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${FREEIMAGE_LIBRARIES}
    OpdeFile
    OpdeLogger
    OpdeConsole
    DarkSceneManager
    ${OPDE_SERVICE_LIBRARIES}
    OpdeBase
)

# Doc generator for opde - property and link types info generator
add_executable (opdeDocGen WIN32 
    OpdeDocGen.cpp
)

# Main executable
add_executable (opde WIN32 
    Opde.cpp 
    GameStateManager.cpp
    GameState.cpp
    GamePlayState.cpp
    GameLoadState.cpp
    DTypeScriptCompiler.cpp
    DTypeScriptLoader.cpp
    PLDefScriptCompiler.cpp
    PLDefScriptLoader.cpp
    GameStateManager.h
    GameState.h
    GamePlayState.h
    GameLoadState.h
    DTypeScriptCompiler.h
    DTypeScriptLoader.h
    PLDefScriptCompiler.h
    PLDefScriptLoader.h
    CustomImageCodec.cpp
    CustomImageCodec.h
)



target_link_libraries( opde
    OpdeLib
)

target_link_libraries( opdeDocGen
    OpdeLib
)

