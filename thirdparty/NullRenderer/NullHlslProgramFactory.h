#ifndef __NULLHLSLPROGRAMFACTORY_H
#define __NULLHLSLPROGRAMFACTORY_H

#include "stdafx.h"

namespace Ogre {
	class NULLHLSLProgramFactory : public Ogre::HighLevelGpuProgramFactory
	{
		public:
			const String& getLanguage() const;
			HighLevelGpuProgram* create(ResourceManager* creator, const Ogre::String& name, ResourceHandle handle,
					const Ogre::String& group, bool isManual, Ogre::ManualResourceLoader* loader);
			void destroy(Ogre::HighLevelGpuProgram*);
			
		protected:
			static const String msLanguage;
	};
}

#endif
