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
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/thirdparty/QuickGUI/include
    ${OPDE_SOURCE_DIR}/src/main
)

add_executable(pytest 
    test.cpp 
    test.h 
    ../../src/main/DTypeScriptLoader.cpp
    ../../src/main/DTypeScriptLoader.h
    ../../src/main/PLDefScriptLoader.cpp
    ../../src/main/PLDefScriptLoader.h
    ../../src/main/DTypeScriptCompiler.cpp
    ../../src/main/DTypeScriptCompiler.h
    ../../src/main/PLDefScriptCompiler.cpp
    ../../src/main/PLDefScriptCompiler.h
)


target_link_libraries(pytest 
    ${OPDE_BASE_LIBRARIES}
    ${OPDE_SERVICE_LIBRARIES}
    OpdePythonBindings
)
