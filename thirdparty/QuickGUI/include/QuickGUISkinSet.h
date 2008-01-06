#ifndef QUICKGUIIMAGESET_H
#define QUICKGUIIMAGESET_H

#include "OgreCodec.h"
#include "OgreException.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreMaterialManager.h"
#include "OgrePrerequisites.h"
#include "OgreResourceGroupManager.h"
#include "OgreTextureManager.h"
#include "OgreVector4.h"
#include "OgreLogManager.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIExportDLL.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

namespace QuickGUI
{
	/** 
	* Stores a generated SkinSet from the individual widget images
	*/
	class _QuickGUIExport SkinSet
	{
	public:
		friend class SkinSetManager;

		enum ImageType
		{
			IMAGE_TYPE_PNG		=  0,
			IMAGE_TYPE_JPG			,
			IMAGE_TYPE_BMP			,
			IMAGE_TYPE_PCX
		};
	public:
		/*
		* Adds a texture name to the list of textures that will be included in the
		* SkinSet texture.  
		* NOTE: Texture will not be present until buildTexture function is executed.
		*/
		void addTexture(const Ogre::String& textureName, const Ogre::Vector4 &texCoord = Ogre::Vector4::ZERO);

		/*
		* Removes a texture name to the list of textures that will be included in the
		* SkinSet texture.  
		* NOTE: Texture will not be removed until buildTexture function is executed.
		*/
		void removeTexture(const Ogre::String& textureName);

		/*
		* Builds the resulting Image from all added Images.  If Texture has already
		* been built, it will be rebuilt.  If you have added images to the SkinSet,
		* they will not be in the texture until you call this function.
		*/
		void buildTexture();

		/*
		* Compute texture coordinates inside the Skinset for each texture.
		*/
		void buildTextureCoordinates(std::vector<Ogre::Image> &images);

		/*
		* Returns true if SkinSet Texture contains texture, false otherwise.
		*/
		bool containsImage(Ogre::String textureName);

		Ogre::String getImageExtension() const;
		Ogre::String getSkinName() const;
		// Return the name of the skin for this SkinSet
		Ogre::String getTextureName() const;

		/*
		* Return the UV coordinates of the image, assuming the image is a part of the image set.
		* Return form is (left,top,right,bot). (left and right refer to x-coordinate, top and
		* bottom refer to y-coordinates)
		*/
		Ogre::Vector4 getTextureCoordinates(const Ogre::String &imageName) const;
		/*
		* set the UV coordinates of the image, assuming the image is a part of the image set.
		* Vector4 form is (left,top,right,bot). (left and right refer to x-coordinate, top and
		* bottom refer to y-coordinates)
		*/
		void setTextureCoordinates(const Ogre::String &imageName, const Ogre::Vector4 &texCoord);
		/*
		* saves skin to a .skinset file in the folder containing first 
		* place found in the resource group.
		*/
		void saveSkin();
		/*
		* loads a skin from a .skinset file return false if failing
		*/
		bool loadSkin();

		/*
		* Retrieve material used for this Skinset.
		*/
		Ogre::String getMaterialName() { return mMaterialName; }
		/*
		* set material used for this Skinset.
		*/
		void setMaterialName(const Ogre::String& materialName){ mMaterialName = materialName; }

		const Ogre::StringVector &getTextureNames() const {return mTextureNames;}


		/*
		* Appends mapping of imagename -> filename
		*/
		void setImageMapping(const Ogre::String& imageName, const Ogre::String& fileName, bool subscope);

	protected:
		Ogre::String mSkinName;

		// Corresponding to the Image Type. (IMAGE_TYPE_PNG -> ".png")
		Ogre::String mImageExtension;

		// list of textures used to build the SkinSet Texture.
		Ogre::StringVector mTextureNames;
		// list of textures that are currently in the SkinSet Texture.
		std::set<Ogre::String> mContainedTextures;

		// The actual texture that stores the imageset
		Ogre::uint mTextureWidth;
		Ogre::uint mTextureHeight;

		// This map connects individual textures to its UV coordinates, within
		// the SkinSet Texture.
		std::map<Ogre::String,Ogre::Vector4> mTextureMap;

		// Number of images that this SkinSet has
		size_t mNumIndividualTextures;
		
		typedef std::map< Ogre::String, Ogre::String > ImageFileMap;
		
		// Map for whole subscope
		ImageFileMap mSubscopeMap;
		// Map for one specific case (Higher priority)
		ImageFileMap mImageMap;
		
		// Unmaps the image name to the file name (or leaves value as is if mapping not found)
		const Ogre::String& unmapImageFile(const Ogre::String& imageName) const;

	private:
		// Generate a new SkinSet using the skin's image files.
		SkinSet(const Ogre::String& skinName, ImageType t, const Ogre::String &resourceGroup, bool loadOnCreate = true);
		// Delete this SkinSet
		~SkinSet();

		void _determineExtension(ImageType t);
		void buildMaterial();
		void _findSkinTextures();
		Ogre::String mTextureName;
		Ogre::String mResourceGroup;
		Ogre::String mMaterialName;
		bool mDirtyTexture;
		bool mDirtyTextureCoordinates;
	};
}

#endif
