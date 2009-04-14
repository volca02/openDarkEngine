# $Id$

include_directories(
    ${OGRE_INCLUDE_DIR}
)

add_library (NullRenderSystem SHARED
	NullGpuProgram.h
	NullGpuProgramManager.cpp
	NullGpuProgramManager.h
	NullHardwareBufferManager.cpp
	NullHardwareBufferManager.h
	NullHardwareIndexBuffer.cpp
	NullHardwareIndexBuffer.h
	NullHardwareVertexBuffer.cpp
	NullHardwareVertexBuffer.h
	NullHlslProgramFactory.cpp	
	NullHlslProgramFactory.h
	NullRendererDll.cpp
	NullRenderSystem.cpp
	NullRenderSystem.h
	NullRenderWindow.cpp
	NullRenderWindow.h
	NullTexture.h
	NullTextureManager.cpp
	NullTextureManager.h
	stdafx.h
)

target_link_libraries( NullRenderSystem
    ${OGRE_LIBRARIES}
)

set_property(TARGET OpenDark PROPERTY LINK_INTERFACE_LIBRARIES "")

# Build flags for the .DLL __declspec things, etc.
SET_TARGET_PROPERTIES(NullRenderSystem PROPERTIES
  DEFINE_SYMBOL "OPDELIB_DLL_TARGET"
  LINK_INTERFACE_LIBRARIES ""
)
