#include "stdafx.h"
#include "NullHlslProgramFactory.h"

namespace Ogre {
	const String NULLHLSLProgramFactory::msLanguage = "NULLHLSL";

	const String& NULLHLSLProgramFactory::getLanguage() const {
		return msLanguage;
	}
	
	HighLevelGpuProgram* NULLHLSLProgramFactory::create(ResourceManager* creator, const Ogre::String& name, ResourceHandle handle,
			const Ogre::String& group, bool isManual, Ogre::ManualResourceLoader* loader) {
	}
	
	void NULLHLSLProgramFactory::destroy(Ogre::HighLevelGpuProgram* prg) {
	}
};

