# Nothing interesting now. Will compile the python module after finished
#
# $Id: CMakeLists.txt 478 2007-12-21 12:08:43Z volca $

INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

include_directories(
    ${OGRE_INCLUDE_DIR}
    ${OIS_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
    ${PYTHON_INCLUDE_PATH}
    ${OPDE_SOURCE_DIR}/src/bindings
)

add_executable(pytest test.cpp test.h)


target_link_libraries(pytest 
    ${OPDE_BASE_LIBRARIES}
    ${OPDE_SERVICE_LIBRARIES}
    OpdePythonBindings
)
