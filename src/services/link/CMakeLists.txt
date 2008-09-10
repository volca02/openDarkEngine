INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

include_directories ( 
    ${OGRE_INCLUDE_DIR} 
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
)

link_directories ( 
    ${OGRE_LIB_DIR}
    ${OPDE_SOURCE_DIR}/src/base
    ${OPDE_SOURCE_DIR}/src/base/servicemanager
)

add_library (OpdeLinkService
    LinkService.cpp
    Relation.cpp
    LinkService.h
    Relation.h
    LinkCommon.h
)

#target_link_libraries(OpdeLinkService
#    OpdeBase
#    OpdeServiceManager
#    ${OPDE_BASE_LIBRARIES}
#    ${OPDE_SERVICE_LIBRARIES}
#)
