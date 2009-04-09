#include "stdafx.h"
#include "NullHardwareVertexBuffer.h"

namespace Ogre {

	//---------------------------------------------------------------------
    NULLHardwareVertexBuffer::NULLHardwareVertexBuffer(size_t vertexSize, 
        size_t numVertices, HardwareBuffer::Usage usage, 
        bool useSystemMemory, bool useShadowBuffer)
        : HardwareVertexBuffer(vertexSize, numVertices, usage, useSystemMemory, useShadowBuffer)
    {
        // Create the vertex buffer
		m_pBuffer = (char *)malloc(vertexSize * numVertices);

    }
	//---------------------------------------------------------------------
    NULLHardwareVertexBuffer::~NULLHardwareVertexBuffer()
    {
		free(m_pBuffer);
    }
	//---------------------------------------------------------------------

//---------------------------------------------------------------------
    void* NULLHardwareVertexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        return &m_pBuffer[offset];
    }
	//---------------------------------------------------------------------
	void NULLHardwareVertexBuffer::unlockImpl(void)
    {
    }
	//---------------------------------------------------------------------
    void NULLHardwareVertexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
		memcpy(pDest, &m_pBuffer[offset], length);
    }
	//---------------------------------------------------------------------
    void NULLHardwareVertexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource,
			bool discardWholeBuffer)
    {
		memcpy(&m_pBuffer[offset], pSource, length);
    }
	//---------------------------------------------------------------------

}
