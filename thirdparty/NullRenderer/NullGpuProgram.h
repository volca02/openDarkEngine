#ifndef __NULLGPUPROGRAM_H
#define __NULLGPUPROGRAM_H

#include "stdafx.h"

namespace Ogre {

	class NULLGpuProgram : public GpuProgram
	{
    		protected:
    		public:
       			NULLGpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
		   		const String& group, bool isManual = false, ManualResourceLoader* loader = 0)  : GpuProgram(creator, name, handle, group, isManual, loader) {}
		
    		protected:
        		/** @copydoc Resource::loadImpl */
			void loadImpl(void) {}
        		
			/** Overridden from GpuProgram */
			void loadFromSource(void) {}
			
			/** Internal method to load from microcode, must be overridden by subclasses. */
			void unloadImpl(void) {}

    };

}

#endif
