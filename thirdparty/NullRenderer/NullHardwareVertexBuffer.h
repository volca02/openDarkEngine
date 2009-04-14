/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *	  $Id$
 *
 *****************************************************************************/

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
