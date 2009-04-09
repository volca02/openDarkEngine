#include "stdafx.h"
#include "NullHardwareIndexBuffer.h"

namespace Ogre {

	NULLHardwareIndexBuffer::NULLHardwareIndexBuffer(HardwareIndexBuffer::IndexType idxType, 
        size_t numIndexes, HardwareBuffer::Usage usage, bool useSystemMemory, bool useShadowBuffer)
        : HardwareIndexBuffer(idxType, numIndexes, usage, useSystemMemory, useShadowBuffer)
    {
		m_pBuffer = (char *)malloc(numIndexes * sizeof(Ogre::uint32));
    }
	//---------------------------------------------------------------------
    NULLHardwareIndexBuffer::~NULLHardwareIndexBuffer()
    {
		free(m_pBuffer);
    }
	//---------------------------------------------------------------------
    void* NULLHardwareIndexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        return &m_pBuffer[offset];
    }
	//---------------------------------------------------------------------
	void NULLHardwareIndexBuffer::unlockImpl(void)
    {
    }
	//---------------------------------------------------------------------
    void NULLHardwareIndexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
		memcpy(pDest, &m_pBuffer[offset], length);
    }
	//---------------------------------------------------------------------
    void NULLHardwareIndexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource,
			bool discardWholeBuffer)
    {
		memcpy(&m_pBuffer[offset],pSource, length);
	}

}
