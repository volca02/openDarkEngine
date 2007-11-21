include_directories( 
	${OGRE_INCLUDE_DIR} 
	${OPDE_BINARY_DIR}
)

link_directories (
    ${OGRE_LIB_DIR}
)

file(GLOB OPDE_DARKSCENEMANAGER_HEADERS *.h)

add_library (DarkSceneManager 
		OgreBspNode.cpp 
		OgreBspTree.cpp 
		OgreDarkSceneManager.cpp 
		OgreDarkSceneNode.cpp
		OgrePolygon.cpp
		OgrePortal.cpp
		OgrePortalFrustum.cpp
		${OPDE_DARKSCENEMANAGER_HEADERS}
	)