include_directories ( 
    ${OPDE_SOURCE_DIR}/src/base
)

add_library(OpdeLogger logger.cpp stdlog.cpp)
