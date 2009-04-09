#include "stdafx.h"
#include "NullHardwareBufferManager.h"
#include "NullHardwareIndexBuffer.h"
#include "NullHardwareVertexBuffer.h"
#include "NullRenderToVertexBuffer.h"

namespace Ogre {

	HardwareVertexBufferSharedPtr NULLHardwareBufferManager::createVertexBuffer(
			size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage,
			bool useShadowBuffer) {
		HardwareVertexBuffer* vbuf = new NULLHardwareVertexBuffer(vertexSize,
				numVerts, usage, false, false);
		mVertexBuffers.insert(vbuf);
	
		return HardwareVertexBufferSharedPtr(vbuf);
	}
	
	HardwareIndexBufferSharedPtr NULLHardwareBufferManager::createIndexBuffer(
			HardwareIndexBuffer::IndexType itype, size_t numIndexes,
			HardwareBuffer::Usage usage, bool useShadowBuffer) {
		NULLHardwareIndexBuffer* idx = new NULLHardwareIndexBuffer(itype,
				numIndexes, usage, false, useShadowBuffer);
		mIndexBuffers.insert(idx);
	
		return HardwareIndexBufferSharedPtr(idx);
	
	}
	
	RenderToVertexBufferSharedPtr NULLHardwareBufferManager::createRenderToVertexBuffer() {
		return RenderToVertexBufferSharedPtr(new NULLRenderToVertexBuffer());
	}

}

