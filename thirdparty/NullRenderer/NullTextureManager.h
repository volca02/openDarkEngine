#ifndef __NULLTEXTUREMANAGER_H
#define __NULLTEXTUREMANAGER_H

#include "stdafx.h"

namespace Ogre {

	class NULLTextureManager : public TextureManager
	{
		protected:
			/// @copydoc ResourceManager::createImpl
			Resource* createImpl(const String& name, ResourceHandle handle, 
				const String& group, bool isManual, ManualResourceLoader* loader, 
				const NameValuePairList* createParams);
	
		public:
			NULLTextureManager();
			~NULLTextureManager();
	
			/// @copydoc TextureManager::getNativeFormat
			PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage);
	
			virtual bool isHardwareFilteringSupported (TextureType ttype, PixelFormat format, int usage, bool preciseFormatOnly=false);
	};

}

#endif
