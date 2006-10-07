include_directories( ${OGRE_INCLUDE_DIR} )

add_library (DarkSceneManager OgreBspNode.cpp OgreBspTree.cpp OgreDarkSceneManager.cpp OgreDarkSceneNode.cpp OgrePortal.cpp OgrePortalFrustum.cpp)