include_directories( 
	${OGRE_INCLUDE_DIR} 
	${OPDE_SOURCE_DIR}
)

link_directories (
    ${OGRE_LIB_DIR}
)

add_library (DarkSceneManager 
		OgreBspNode.cpp 
		OgreBspTree.cpp 
		OgreDarkSceneManager.cpp 
		OgreDarkSceneNode.cpp
		OgrePolygon.cpp
		OgrePortal.cpp
		OgrePortalFrustum.cpp
	)