include_directories( 
	${OGRE_INCLUDE_DIR} 
	${OPDE_BINARY_DIR}
)

link_directories (
    ${OGRE_LIB_DIR}
)

add_library (DarkSceneManager 
		DarkBspNode.cpp 
		DarkBspNode.h
		DarkBspTree.cpp 
		DarkBspTree.h
		DarkSceneNode.cpp
		DarkSceneNode.h
		DarkPolygon.cpp
		DarkPolygon.h
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
	)