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
    ${OPDE_SOURCE_DIR}/src/services/render
    ${OPDE_SOURCE_DIR}/src/services/database
    ${OPDE_SOURCE_DIR}/src/services/input
    ${OPDE_SOURCE_DIR}/src/services/loop
)

# All the resulting libraries in a nice package as well
SET(OPDE_SERVICE_LIBRARIES
    OpdeServiceManager
    OpdeWorldRepService
    OpdeBinaryService
    OpdeGameService
    OpdeConfigService
    OpdeLinkService
    OpdePropertyService
    OpdeInheritService
    OpdeRenderService
    OpdeDatabaseService
    OpdeInputService
    OpdeLoopService
)

# To use this, just do INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)
