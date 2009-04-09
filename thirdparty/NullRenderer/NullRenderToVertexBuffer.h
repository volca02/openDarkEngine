#ifndef __NULLRENDERTOVERTEXBUFFER_H
#define __NULLRENDERTOVERTEXBUFFER_H

#include "stdafx.h"

namespace Ogre {

	class NULLRenderToVertexBuffer : public RenderToVertexBuffer {
		public:
			NULLRenderToVertexBuffer() {};
			
		
			virtual void getRenderOperation(Ogre::RenderOperation& op) {};
			virtual void update(Ogre::SceneManager*) {};
	};

}

#endif
