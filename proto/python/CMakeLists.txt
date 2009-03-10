# Nothing interesting now. Will compile the python module after finished
#
# $Id$

INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

include_directories(
    ${ODE_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}    
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
    ${PYTHON_INCLUDE_PATH}
    ${OPDE_SOURCE_DIR}/src/bindings
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/thirdparty/QuickGUI/include
    ${OPDE_SOURCE_DIR}/src/main
    ${INCLUDE_ADDITION_CUSTOM_IMAGE}
)

add_executable(pytest 
    test.cpp 
    test.h 
)


target_link_libraries(pytest 
    OpenDark
    ${OPDE_PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES( pytest PROPERTIES
  COMPILE_DEFINITIONS "OPDE_EXE_TARGET"
)

