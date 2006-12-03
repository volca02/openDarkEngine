# include_directories (${HELLO_SOURCE_DIR}/Hello)
# link_directories (${HELLO_BINARY_DIR}/Hello)
# target_link_libraries (helloDemo Hello)

include_directories(
    ${OGRE_INCLUDE_DIR}
    ${CEGUI_INCLUDE_DIR}
    ${OPDE_SOURCE_DIR}
    ${OPDE_SOURCE_DIR}/src/compat
    ${OPDE_SOURCE_DIR}/src/base/logger
    ${OPDE_SOURCE_DIR}/src/base/gui
    ${OPDE_SOURCE_DIR}/src/base/console
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/base/Dark
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/main
    ${OPDE_SOURCE_DIR}/src/widgets
    ${OPDE_SOURCE_DIR}/src/services/binary
    ${OPDE_SOURCE_DIR}/src/services/database
    ${OPDE_SOURCE_DIR}/src/services/worldrep
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

add_executable (opde WIN32 Opde.cpp OpdeMission.cpp)

target_link_libraries( opde
    ${OGRE_LIBRARIES}
    ${CEGUI_LIBRARIES}
#    ${EXPAT_LIBRARIES}
    OpdeLogger
    OpdeConsole
    OpdeGUI
    OpdeWidgetConsole
    OpdeServiceManager
    OpdeWorldRepService
    DarkSceneManager
    OpdeDatabaseService
#    OpdeBinaryService
    OpdeBase
)

