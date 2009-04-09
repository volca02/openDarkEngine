#include "stdafx.h"
#include "NullTextureManager.h"
#include "NullTexture.h"

namespace Ogre {

	NULLTextureManager::NULLTextureManager(  ) : TextureManager()
	{
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
	}

	NULLTextureManager::~NULLTextureManager()
	{
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

	}

    Resource* NULLTextureManager::createImpl(const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        return new NULLTexture(this, name, handle, group, isManual, loader); 
    }


	PixelFormat NULLTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
	{
		return PF_A8;
	}
	
	bool NULLTextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage, bool preciseFormatOnly) {
		return true;
	}

}
