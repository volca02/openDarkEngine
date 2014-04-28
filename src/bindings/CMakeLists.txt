# Nothing interesting now. Will compile the python module after finished
#
# $Id$

INCLUDE( ${OPDE_SOURCE_DIR}/src/services/Services.cmake )
INCLUDE( ${OPDE_SOURCE_DIR}/src/base/Base.cmake )

include_directories(
    ${ODE_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
    ${OPDE_BASE_INCLUDES}
    ${OPDE_SERVICE_INCLUDES}
    ${PYTHON_INCLUDE_PATH}
    ${OPDE_SOURCE_DIR}/src/scenemanager
    ${OPDE_SOURCE_DIR}/src/main
)

IF(STATIC_LIBS)
    add_library(OpdePyLib SHARED
        bindings.cpp 
        bindings.h
        ServiceBinder.cpp
        ServiceBinder.h
        ConfigServiceBinder.cpp
        ConfigServiceBinder.h
        LinkServiceBinder.cpp
        LinkServiceBinder.h
        RelationBinder.cpp
        RelationBinder.h
        LinkQueryResultBinder.cpp
        LinkQueryResultBinder.h
        PropertyServiceBinder.cpp
        PropertyServiceBinder.h
        DTypeBinder.cpp
        DTypeBinder.h
        LoopServiceBinder.cpp
        LoopServiceBinder.h
        InputServiceBinder.cpp
        InputServiceBinder.h
        GUIServiceBinder.cpp
        GUIServiceBinder.h
        DatabaseServiceBinder.cpp
        DatabaseServiceBinder.h
        PythonStruct.h
        RootBinder.cpp
        RootBinder.h
        StringIteratorBinder.cpp
        StringIteratorBinder.h
        ObjectServiceBinder.cpp
        ObjectServiceBinder.h
    )

    target_link_libraries(OpdePyLib
        ${OPDE_PYTHON_LIBRARIES}
        OpenDark
    )
ENDIF(STATIC_LIBS)
