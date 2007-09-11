# All the services as a nice includable script containing the list of them

SET(OPDE_SERVICE_INCLUDES
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
    ${OPDE_SOURCE_DIR}/src/services/
    ${OPDE_SOURCE_DIR}/src/services/binary
    ${OPDE_SOURCE_DIR}/src/services/game
    ${OPDE_SOURCE_DIR}/src/services/worldrep
    ${OPDE_SOURCE_DIR}/src/services/link
    ${OPDE_SOURCE_DIR}/src/services/property
    ${OPDE_SOURCE_DIR}/src/services/inherit
    ${OPDE_SOURCE_DIR}/src/services/render
)

# All the resulting libraries in a nice package as well
SET(OPDE_SERVICE_LIBRARIES
    OpdeServiceManager
    OpdeWorldRepService
    OpdeBinaryService
    OpdeGameService
    OpdeLinkService
    OpdePropertyService
    OpdeInheritService
    OpdeRenderService
)

# To use this, just do INCLUDE(${OPDE_SOURCE_DIR}/src/services/Services.cmake)
