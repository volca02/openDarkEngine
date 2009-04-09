#ifndef __NULLGPUPROGRAMMANAGER_H
#define __NULLGPUPROGRAMMANAGER_H

#include "stdafx.h"

namespace Ogre {

	class NULLGpuProgramManager: public GpuProgramManager {
		protected:
			/// @copydoc ResourceManager::createImpl
			Resource* createImpl(const String& name, ResourceHandle handle,
					const String& group, bool isManual, ManualResourceLoader* loader,
					const NameValuePairList* params);
			/// Specialised create method with specific parameters
			Resource* createImpl(const String& name, ResourceHandle handle,
					const String& group, bool isManual, ManualResourceLoader* loader,
					GpuProgramType gptype, const String& syntaxCode);
		
		public:
			NULLGpuProgramManager();
			~NULLGpuProgramManager();
			/// @copydoc GpuProgramManager::createParameters
			GpuProgramParametersSharedPtr createParameters(void);
	};
}

#endif
