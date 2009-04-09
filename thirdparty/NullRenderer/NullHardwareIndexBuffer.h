#ifndef __NULLHARDWAREINDEXBUFFER_H
#define __NULLHARDWAREINDEXBUFFER_H

#include "stdafx.h"

namespace Ogre {

	class NULLHardwareIndexBuffer: public HardwareIndexBuffer {
	protected:
		/** See HardwareBuffer. */
		void* lockImpl(size_t offset, size_t length, LockOptions options);
		/** See HardwareBuffer. */
		void unlockImpl(void);
	public:
		NULLHardwareIndexBuffer(IndexType idxType, size_t numIndexes,
				HardwareBuffer::Usage usage, bool useSystemMem,
				bool useShadowBuffer);
		~NULLHardwareIndexBuffer();
		/** See HardwareBuffer. */
		void readData(size_t offset, size_t length, void* pDest);
		/** See HardwareBuffer. */
		void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);
	
		char *m_pBuffer;
	};

}

#endif
