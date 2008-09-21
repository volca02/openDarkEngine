INCLUDE(${OPDE_SOURCE_DIR}/src/base/Base.cmake)

add_subdirectory(logger)
add_subdirectory(console)
add_subdirectory(servicemanager)
add_subdirectory(file)
add_subdirectory(dyntype)
add_subdirectory(loaders)

FILE(GLOB OPDE_BASE_HEADERS *.h)

include_directories(
    ${OPDE_BINARY_DIR}
)

add_library ( OpdeBase OpdeException.cpp RefCounted.cpp
    ${OPDE_BASE_HEADERS}
)

SET_TARGET_PROPERTIES(OpdeBase PROPERTIES
  COMPILE_DEFINITIONS "OPDELIB_DLL_TARGET"
)

target_link_libraries( OpdeBase
    ${OPDE_BASE_LIBRARIES}
)
