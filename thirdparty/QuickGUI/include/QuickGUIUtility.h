#ifndef QUICKGUIUTILITY_H
#define QUICKGUIUTILITY_H

#include "OgreCodec.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include "OgreResourceGroupManager.h"
#include "OgreVector4.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIExportDLL.h"

namespace QuickGUI
{
	class _QuickGUIExport Utility
	{
	public:
		Utility();
		~Utility();

		static Ogre::Vector4 getImageBounds(const Ogre::Image& i);

		static bool isImageFile(const Ogre::String& fileName);

		/*
		* Returns true if the textureName represents an image file on disk. (ie *.png, *.jpg, etc)
		*/
		static bool textureExistsOnDisk(const Ogre::String& textureName);

	protected:
		static Ogre::StringVector mSupportedImageCodecs;
	};
}

#endif
