#include "stdafx.h"
#include "NullRenderSystem.h"
#include "NullHlslProgramFactory.h"

namespace Ogre {

	NULLRenderSystem* nullRendPlugin;
	NULLHLSLProgramFactory* hlslProgramFactory;

	extern "C" void dllStartPlugin(void) throw()
	{
		nullRendPlugin = new NULLRenderSystem();
		Ogre::Root::getSingleton().addRenderSystem( (Ogre::RenderSystem *)nullRendPlugin );

        // create & register HLSL factory
        hlslProgramFactory = new NULLHLSLProgramFactory();
		Ogre::HighLevelGpuProgramManager::getSingleton().addFactory(hlslProgramFactory);
	}

	extern "C" void dllStopPlugin(void)
	{
		delete nullRendPlugin;
		delete hlslProgramFactory;
	}

}
