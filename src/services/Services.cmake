# All the services as a nice includable script containing the list of them

SET(OPDE_SERVICE_INCLUDES
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/services/
    ${OPDE_SOURCE_DIR}/src/services/binary
    ${OPDE_SOURCE_DIR}/src/services/game
    ${OPDE_SOURCE_DIR}/src/services/config
    ${OPDE_SOURCE_DIR}/src/services/worldrep
    ${OPDE_SOURCE_DIR}/src/services/link
    ${OPDE_SOURCE_DIR}/src/services/property
    ${OPDE_SOURCE_DIR}/src/services/inherit
    ${OPDE_SOURCE_DIR}/src/services/object
    ${OPDE_SOURCE_DIR}/src/services/render
    ${OPDE_SOURCE_DIR}/src/services/database
    ${OPDE_SOURCE_DIR}/src/services/input
    ${OPDE_SOURCE_DIR}/src/services/loop
    ${OPDE_SOURCE_DIR}/src/services/gui
    ${OPDE_SOURCE_DIR}/src/services/script
)

# All the resulting libraries in a nice package as well
SET(OPDE_SERVICE_LIBRARIES
    OpdeWorldRepService
    OpdeBinaryService
    OpdeGameService
    OpdeConfigService
    OpdeLinkService
    OpdePropertyService
    OpdeInheritService
    OpdeObjectService
    OpdeRenderService
    OpdeDatabaseService
    OpdeInputService
    OpdeLoopService
    OpdeGUIService
    OpdeScriptService
)

# To use this, just do INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)
