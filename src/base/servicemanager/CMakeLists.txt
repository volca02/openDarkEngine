include_directories ( ${OPDE_SOURCE_DIR}/src/base/ ${OGRE_INCLUDE_DIR})
add_library (OpdeServiceManager OpdeServiceManager.cpp OpdeService.cpp)
