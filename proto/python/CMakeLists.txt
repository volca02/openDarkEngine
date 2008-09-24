# Nothing interesting now. Will compile the python module after finished
#
# $Id: CMakeLists.txt 478 2007-12-21 12:08:43Z volca $

INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

SET(LIB_ADDITION_CUSTOM_IMAGE)
SET(INCLUDE_ADDITION_CUSTOM_IMAGE)
# If custom image hooks was selected
if(CUSTOM_IMAGE_HOOKS)
    add_library(OpdeCustomImageCodec 
    ../../src/main/CustomImageCodec.cpp 
    ../../src/main/CustomImageCodec.h
    )

    SET(LIB_ADDITION_CUSTOM_IMAGE OpdeCustomImageCodec ${FREEIMAGE_LIBRARIES})
    SET(INCLUDE_ADDITION_CUSTOM_IMAGE OpdeCustomImageCodec ${FREEIMAGE_INCLUDE_DIR})
endif(CUSTOM_IMAGE_HOOKS)

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
    ${INCLUDE_ADDITION_CUSTOM_IMAGE}
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
    OpdeLib
)
