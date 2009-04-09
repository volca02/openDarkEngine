#ifndef __NULLTEXTURE_H
#define __NULLTEXTURE_H

#include "stdafx.h"

using namespace Ogre;

class NULLTexture : public Texture
{
public:
	NULLTexture(ResourceManager* creator, const String& name, ResourceHandle handle,
		const String& group, bool isManual = false, ManualResourceLoader* loader = 0) : Texture(creator, name, handle, group, isManual, loader) { }
	~NULLTexture() {}
	void loadImpl(void) {}
	void loadImage (const Ogre::Image &) {}
	HardwarePixelBufferSharedPtr getBuffer(size_t face=0, size_t mipmap=0) { return HardwarePixelBufferSharedPtr(NULL); }
	void createInternalResourcesImpl() {}
	void freeInternalResourcesImpl() {}
};

#endif
