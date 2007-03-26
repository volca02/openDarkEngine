include_directories ( 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
)

add_library(OpdeLogger logger.cpp stdlog.cpp)
