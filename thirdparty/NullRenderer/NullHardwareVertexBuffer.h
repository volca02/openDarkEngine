#ifndef __NULLHARDWAREVERTEXBUFFER_H
#define __NULLHARDWAREVERTEXBUFFER_H

#include "stdafx.h"

namespace Ogre {

	class NULLHardwareVertexBuffer : public HardwareVertexBuffer 
		{
		protected:
			/** See HardwareBuffer. */
			virtual void* lockImpl(size_t offset, size_t length, LockOptions options);
			/** See HardwareBuffer. */
			virtual void unlockImpl(void);
		public:
			NULLHardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
				HardwareBuffer::Usage usage, bool useSystemMem, bool useShadowBuffer);
			~NULLHardwareVertexBuffer();
			/** See HardwareBuffer. */
			void readData(size_t offset, size_t length, void* pDest);
			/** See HardwareBuffer. */
			void writeData(size_t offset, size_t length, const void* pSource,
					bool discardWholeBuffer = false);
	
			char *m_pBuffer;
	
		};
}

#endif
