INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

SET(LIB_ADDITION_CUSTOM_IMAGE)
SET(INCLUDE_ADDITION_CUSTOM_IMAGE)
# If custom image hooks was selected
if(CUSTOM_IMAGE_HOOKS)
    add_library(OpdeCustomImageCodec CustomImageCodec.cpp CustomImageCodec.h)
    
    SET(LIB_ADDITION_CUSTOM_IMAGE OpdeCustomImageCodec ${FREEIMAGE_LIBRARIES})
    SET(INCLUDE_ADDITION_CUSTOM_IMAGE OpdeCustomImageCodec ${FREEIMAGE_INCLUDE_DIR})
endif(CUSTOM_IMAGE_HOOKS)

include_directories(
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${INCLUDE_ADDITION_CUSTOM_IMAGE}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/widgets
    ${OPDE_SERVICE_INCLUDES}
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
)



target_link_libraries( opde
    ${OGRE_LIBRARIES}
    ${CEGUI_LIBRARIES}
    ${OIS_LIBRARIES}
    ${LIB_ADDITION_CUSTOM_IMAGE}
    OpdeFile
    OpdeLogger
    OpdeConsole
    DarkSceneManager
    ${OPDE_SERVICE_LIBRARIES}
    OpdeBase
)

