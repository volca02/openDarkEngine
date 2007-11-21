include_directories ( 
    ${OPDE_BINARY_DIR}
    ${OPDE_SOURCE_DIR}/src/base
)

file(GLOB OPDE_LOGGER_HEADERS *.h)

add_library(OpdeLogger logger.cpp stdlog.cpp filelog.cpp ${OPDE_LOGGER_HEADERS})
