#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIUtility.h"

namespace QuickGUI
{
	Utility::Utility()
	{	
	}

	Utility::~Utility()
	{
	}

	Ogre::Vector4 Utility::getImageBounds(const Ogre::Image& i)
	{
		size_t left = i.getWidth();
		size_t right = 0;
		size_t top = i.getHeight();
		size_t bottom = 0;

		const Ogre::PixelBox& pb = i.getPixelBox();

		// Pointers to the pixel data of the bar, and the destination image
		Ogre::uint8* ptr = static_cast<Ogre::uint8*>(pb.data);

		// iterate through pixel by pixel, to determine the min/max bar bounds.
		for( size_t row = 0; row < i.getHeight(); ++row )
		{
			for( size_t col = 0; col < i.getWidth(); col++ )
			{
				// skip R,B,G channels
				ptr += 3;
				// check for non zero alpha value
				if(*ptr++ > 0)
				{
					if( col < left )
						left = col;
					if( col > right )
						right = col;
					if( row < top )
						top = row;
					if( row > bottom )
						bottom = row;
				}
			}
		}

		return Ogre::Vector4(left,top,right,bottom);
	}

	bool Utility::isImageFile(const Ogre::String& fileName)
	{
		Ogre::String::size_type index = fileName.find_last_of('.');
		if( index != Ogre::String::npos )
		{
			Ogre::String extension = fileName.substr(index + 1);
			Ogre::StringUtil::toLowerCase(extension);

			Ogre::StringVector supportedImageCodecs = Ogre::Codec::getExtensions();

			return (std::find(supportedImageCodecs.begin(),supportedImageCodecs.end(),extension) != supportedImageCodecs.end());
		}

		return false;
	}

	bool Utility::textureExistsOnDisk(const Ogre::String& textureName)
	{
		if(textureName.empty())
			return false;

		if(!isImageFile(textureName))
			return false;

		Ogre::ResourceGroupManager* rgm = Ogre::ResourceGroupManager::getSingletonPtr();
			
		Ogre::StringVector resourceGroups = rgm->getResourceGroups();
		Ogre::StringVector::iterator it;
		for( it = resourceGroups.begin(); it != resourceGroups.end(); ++it )
		{
			if(rgm->resourceExists((*it),textureName))
				return true;
		}

		return false;
	}
}
