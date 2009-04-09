#ifndef __NULLGPUPROGRAM_H
#define __NULLGPUPROGRAM_H

#include "stdafx.h"

namespace Ogre {

	class NULLHardwareBufferManager: public HardwareBufferManager {
		HardwareVertexBufferSharedPtr createVertexBuffer(size_t vertexSize,
				size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer =
						false);
		HardwareIndexBufferSharedPtr createIndexBuffer(
				HardwareIndexBuffer::IndexType itype, size_t numIndexes,
				HardwareBuffer::Usage usage, bool useShadowBuffer = false);
	
		RenderToVertexBufferSharedPtr createRenderToVertexBuffer();
	};
	
	
	
}

#endif
