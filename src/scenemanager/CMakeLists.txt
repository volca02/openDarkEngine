include_directories(
    ${OGRE_INCLUDE_DIR}
    ${OPDE_BINARY_DIR}
)

link_directories (
    ${OGRE_LIB_DIR}
)

add_library (DarkSceneManager OBJECT
    DarkBspNode.cpp
    DarkBspNode.h
    DarkBspTree.cpp
    DarkBspTree.h
    DarkSceneNode.cpp
    DarkSceneNode.h
    DarkConvexPolygon.cpp
    DarkConvexPolygon.h
    DarkPortal.cpp
    DarkPortal.h
    DarkPortalFrustum.cpp
    DarkPortalFrustum.h
    DarkCamera.cpp
    DarkCamera.h
    DarkSceneManager.cpp
    DarkSceneManager.h
    DarkLight.cpp
    DarkLight.h
    DarkGeometry.cpp
    DarkGeometry.h
)
