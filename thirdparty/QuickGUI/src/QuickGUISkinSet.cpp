#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUISkinSet.h"
#include "QuickGUIConfigScriptParser.h"

#include "OgreStringConverter.h"

namespace QuickGUI
{
	SkinSet::SkinSet(const Ogre::String& skinName, ImageType t, const Ogre::String &resourceGroup) :
		mSkinName(skinName),
		mResourceGroup(resourceGroup),
		mNumIndividualTextures(0),
		mTextureWidth(1024),
		mTextureHeight(1024),
		mTextureName("SkinSet." + mSkinName + ".png"),
		mMaterialName(mSkinName + "Material"),
		mDirtyTexture(true),
		mDirtyTextureCoordinates(true),
		mImageExtension("")
	{
		_determineExtension(t);

		// if skinset doesn't exists
		if (loadSkin() == false)
		{
			// assemble texture in a single one
			buildTexture();
		}

		if(!Ogre::MaterialManager::getSingleton().resourceExists(mMaterialName))
			buildMaterial();
	}

	SkinSet::~SkinSet()
	{
		// Remove Texture for this SkinSet from Texture Manager
		if(Ogre::TextureManager::getSingletonPtr()->resourceExists(mTextureName))
			Ogre::TextureManager::getSingletonPtr()->remove(mTextureName);

		if(Ogre::MaterialManager::getSingletonPtr()->resourceExists(mMaterialName))
			Ogre::MaterialManager::getSingletonPtr()->remove(mMaterialName);
	}

	void SkinSet::_determineExtension(ImageType t)
	{
		switch(t)
		{
		case IMAGE_TYPE_PNG:	mImageExtension = ".png";		break;
		case IMAGE_TYPE_JPG:	mImageExtension = ".jpg";		break;
		}
	}

	bool SkinSet::loadSkin()
	{
		ConfigNode *skinRootNode = ConfigScriptLoader::getSingleton().getConfigScript("skinset", mSkinName);
		if (skinRootNode)
		{
			ConfigNode *size = skinRootNode->findChild("size");
			if (size)
			{
				mTextureWidth = Ogre::StringConverter::parseInt(size->getValues()[0]);
				mTextureHeight = Ogre::StringConverter::parseInt(size->getValues()[1]);
			}

			ConfigNode *texName = skinRootNode->findChild("texture");	
			// If the corresponding skinset image file is not available, return.
			Ogre::StringVectorPtr results = Ogre::ResourceGroupManager::getSingleton().findResourceNames(mResourceGroup, mTextureName);
			if(results.isNull() || results->empty())
				return false;

			std::vector<ConfigNode*> children = skinRootNode->getChildren();
			std::vector<ConfigNode*>::iterator it;
			for (it = children.begin(); it != children.end(); ++it)
			{
				ConfigNode* currNode = *it;
				if (currNode->getName() == "element")
				{
					const Ogre::String elementName = currNode->getValues()[0];
					ConfigNode *dimension = currNode->findChild("dimension");
					Ogre::Vector4 texCoord;
					if (dimension)
					{
						texCoord.x = Ogre::StringConverter::parseReal(dimension->getValues()[0]);
						texCoord.y = Ogre::StringConverter::parseReal(dimension->getValues()[1]);
						texCoord.z = Ogre::StringConverter::parseReal(dimension->getValues()[2]);
						texCoord.w = Ogre::StringConverter::parseReal(dimension->getValues()[3]);
					}

					//ConfigNode *rotation = skinRootNode->findChild("rotation");
					//if (rotation)
					//{
					//   rotation.x = Ogre::StringConverter::parseReal(rotation->getValues()[0]);
					//   rotation.y = Ogre::StringConverter::parseReal(rotation->getValues()[1]);
					//   rotation.z = Ogre::StringConverter::parseReal(rotation->getValues()[2]);
					//}

					const Ogre::String texName (mSkinName + "." + elementName + ".png");

					mTextureMap[texName] = texCoord;
					mContainedTextures.insert(texName);
					mTextureNames.push_back(texName);
				}
			}
			mDirtyTexture = false;
			mDirtyTextureCoordinates = false;
			return true;
		}
		return false;
	}

	void SkinSet::saveSkin()
	{
		ConfigScriptSerializer serializer;
		serializer.writeAttribute(0, "skinset " + mSkinName);
		serializer.beginSection(0);
		{
			{
				serializer.writeAttribute(1, "size ");
				{
					serializer.writeValue(Ogre::StringConverter::toString(mTextureWidth));
					serializer.writeValue(Ogre::StringConverter::toString(mTextureHeight));
				}
			}

			{
				serializer.writeAttribute(1, "texture ");
				{
					serializer.writeValue(mTextureName);
				}
			}

			std::map<Ogre::String, Ogre::Vector4>::iterator it;
			for (it = mTextureMap.begin(); it != mTextureMap.end(); ++it)
			{
				const Ogre::String texName (it->first);

				Ogre::StringVector nameParts = Ogre::StringUtil::split(texName, ".");
				Ogre::String elementName;
				Ogre::StringVector::iterator itTexName;
				if (nameParts.size() > 2)
				{
					elementName = nameParts[1];
					for (itTexName = nameParts.begin() + 2; itTexName < nameParts.end() - 1; ++itTexName)
						elementName = elementName +  "." + *itTexName;// sure some clever STL would do this better.

				}
				serializer.writeAttribute(1, "element " + elementName);
				serializer.beginSection(1);
				{
					serializer.writeAttribute(2, "dimension ");
					{
						const Ogre::Vector4 val (it->second);
						serializer.writeValue(Ogre::StringConverter::toString(val.x));
						serializer.writeValue(Ogre::StringConverter::toString(val.y));
						serializer.writeValue(Ogre::StringConverter::toString(val.z));
						serializer.writeValue(Ogre::StringConverter::toString(val.w));
					}
				}
				serializer.endSection(1);
			}
		}
		serializer.endSection(0);

		// export saved result.
		{
			// Saves SkinSet texture
			Ogre::Image finalImageSet;
			// prepare image data.
			{
				const size_t buffSize = mTextureWidth*mTextureHeight*4;
				unsigned char *data = new unsigned char[buffSize];
				finalImageSet.loadDynamicImage(data, mTextureWidth, mTextureHeight, 1, Ogre::PF_R8G8B8A8, true);
				assert (!Ogre::TextureManager::getSingleton().getByName(mTextureName).isNull());
				Ogre::TexturePtr texturePtr = Ogre::TextureManager::getSingleton().getByName(mTextureName);
				Ogre::HardwarePixelBufferSharedPtr buf = texturePtr->getBuffer();
				Ogre::Image::Box textureRect(0, 0, 0, 0, 0, 1);
				textureRect.right = mTextureWidth;
				textureRect.bottom = mTextureHeight;
 				const Ogre::PixelBox destBox = finalImageSet.getPixelBox();
 				//const Ogre::PixelBox srcBox  = buf->lock(textureRect, Ogre::HardwareBuffer::HBL_READ_ONLY); 				
				//Ogre::PixelUtil::bulkPixelConversion(srcBox, destBox); 
				buf->blitToMemory(destBox);
				//buf->unlock();  
			}
			// find where to store it.
			Ogre::String resourcePath;
			{
				Ogre::ResourceGroupManager* rgm = Ogre::ResourceGroupManager::getSingletonPtr();
				Ogre::StringVector resourceGroupNames = rgm->getResourceGroups();

				Ogre::StringVector::iterator groupItr;
				for( groupItr = resourceGroupNames.begin(); groupItr != resourceGroupNames.end(); ++groupItr )
				{
					Ogre::FileInfoListPtr files = rgm->findResourceFileInfo((*groupItr),mSkinName + "*");

					if(!files->empty()) 
					{
						const Ogre::FileInfo &fileInfo = *(files->begin());
						resourcePath = fileInfo.archive->getName();
						break;
					}
				}
			}
			resourcePath = resourcePath + '/';
			// Store for further uses.
			serializer.exportQueued(resourcePath + mSkinName + ".skinset");
			finalImageSet.save(resourcePath + mTextureName);	
		}		
	}

	void SkinSet::_findSkinTextures()
	{
		mTextureNames.clear();

		Ogre::ResourceGroupManager* rgm = Ogre::ResourceGroupManager::getSingletonPtr();
		Ogre::StringVector resourceGroupNames = rgm->getResourceGroups();

		Ogre::StringVector::iterator groupItr;
		for( groupItr = resourceGroupNames.begin(); groupItr != resourceGroupNames.end(); ++groupItr )
		{
			Ogre::FileInfoListPtr files = rgm->findResourceFileInfo((*groupItr),mSkinName + "*" + mImageExtension);

			for( Ogre::FileInfoList::iterator fileItr = files->begin(); fileItr != files->end(); ++fileItr ) 
			{
				Ogre::String fileName = (*fileItr).filename;
				mTextureNames.push_back(fileName);
			}
		}
	}

	void SkinSet::addTexture(const Ogre::String& textureName, const Ogre::Vector4 &texCoord)
	{
		// make sure texture is not already in the list.	
		if (std::find(mTextureNames.begin(), mTextureNames.end(), textureName) == mTextureNames.end())
			return;

		// make sure textureName refers to a valid resource on disk.
		try
		{
			Ogre::ResourceGroupManager::getSingleton().findGroupContainingResource(textureName);
		}
		catch(...) { return; }

		// add it to the list of textures to be used to create SkinSet texture!
		mTextureNames.push_back(textureName);

		if (texCoord != Ogre::Vector4::ZERO)
			mTextureMap[textureName] = texCoord;
		else
			mDirtyTextureCoordinates = true;
		mDirtyTexture = true;
	}
	
	void SkinSet::removeTexture(const Ogre::String& textureName)
	{
		// make sure texture is already in the list.
		Ogre::StringVector::iterator texNameItr = std::find(mTextureNames.begin(), mTextureNames.end(), textureName);
		assert (texNameItr != mTextureNames.end());
		mTextureNames.erase(texNameItr);

		std::set<Ogre::String>::iterator containedTexItr = std::find(mContainedTextures.begin(), mContainedTextures.end(), textureName);
		assert(containedTexItr != mContainedTextures.end());
		mContainedTextures.erase(containedTexItr);


		std::map<Ogre::String,Ogre::Vector4>::iterator itTexMap = mTextureMap.find(textureName);
		assert (itTexMap != mTextureMap.end());
		mTextureMap.erase(itTexMap);
		
		mDirtyTexture = true;
		
	}

	
	void SkinSet::buildTextureCoordinates(std::vector<Ogre::Image> &images)
	{
		// TODO: (optimization) Use pointer in the "images" vector, as each sort iteration means many whole image object copy
		
		// Sort images and names according to Height (Insertion Sort Algorithm)
		// This helps make the final texture more efficient (i.e. densely packed)
		for(size_t i = 1; i < mNumIndividualTextures; i++ )
		{
			Ogre::Image img = images.at(i);
			Ogre::String name = mTextureNames.at(i);
			size_t j = i;
			while( j-- > 0 && images.at(j).getHeight() > img.getHeight() )
			{
				images.at(j+1) = images.at(j);
				mTextureNames.at(j+1) = mTextureNames.at(j);
			}
			images.at(j+1) = img;
			mTextureNames.at(j+1) = name;
		}

		// Create Texture Coordinates for each images inside Atlas Image (Skinset)
		// Minimum of 1 pixel between each image at all times
		mTextureMap.clear();

		const float invTextureWidth = 1.0f / mTextureWidth;
		const float invTextureHeight =  1.0f /mTextureHeight;

		size_t cur_x = 0, cur_y = 0, next_y = 0;
		for(size_t i = 0; i < mNumIndividualTextures; i++)
		{
			const Ogre::Image &img = images.at(i);

			// See if we can still draw on this row, or if we need a new one
			if( (cur_x + img.getWidth()) >= mTextureWidth )
			{
				// Reset variables for drawing the next row
				cur_x = 0;
				cur_y = next_y;
			}

			// Keep track of tallest image in the row
			if( next_y < img.getHeight() + cur_y )
				next_y = img.getHeight() + cur_y + 1;

			// Not enough room for all the images!
			if( next_y >  mTextureHeight  )
			{
				throw Ogre::Exception(1, "The texture" + mTextureNames.at(i)  + 
					" is not large enough to fit all widget images.  Please view \"" + mTextureName + "\"", 
					"QuickGUIImageSet");
			}

			// Store the image name associated with its Texture Coordinates (UV).  
			// UV coordinates are float values between 0 and 1.
			const Ogre::Vector4 dimension(
				cur_x * invTextureWidth, 
				cur_y * invTextureHeight, 
				(img.getWidth() + cur_x)  * invTextureWidth, 
				(img.getHeight() + cur_y) * invTextureHeight 
				);
			mTextureMap[mTextureNames.at(i)] = dimension;

			// Shift x over the correct amount
			cur_x += img.getWidth() + 1;			
		}
		mDirtyTextureCoordinates = false;
	}

	void SkinSet::buildTexture()
	{
		// find textures that will make the skinset
		_findSkinTextures();

		if(Ogre::TextureManager::getSingleton().resourceExists(mTextureName))
		{
			OGRE_EXCEPT(3, "Trying to build SkinSet Texture when it already exists!", "SkinSet::buildTexture()");
			return;
		}

		mContainedTextures.clear();

		// TODO: (optimization) Use pointer in the "images" vector, as each sort iteration means many whole image object copy

		mNumIndividualTextures = 0;
		// Load up each image into a vector
		std::vector<Ogre::Image> images;
		Ogre::StringVector::iterator texNameItr;
		for( texNameItr = mTextureNames.begin(); texNameItr != mTextureNames.end(); ++texNameItr )
		{
			Ogre::Image tempImg;
			Ogre::LogManager::getSingletonPtr()->logMessage("Quickgui : Adding  " + (*texNameItr) + "to skin" + mSkinName);

			tempImg.load((*texNameItr),Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			images.push_back(tempImg);
			mContainedTextures.insert((*texNameItr));

			++mNumIndividualTextures;
		}

		if (mTextureMap.empty() || mDirtyTextureCoordinates)
			buildTextureCoordinates(images);

		// TODO 
		// optimization  : create the atlas texture CPU side and upload to GPU once finished. Much faster.

		// Initialize Texture
		Ogre::TexturePtr texturePtr = Ogre::TextureManager::getSingletonPtr()->createManual(
				mTextureName,
				Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				Ogre::TEX_TYPE_2D,
				mTextureWidth,
				mTextureHeight,
				0,
				Ogre::PF_B8G8R8A8);

		// Texture buffer
		Ogre::HardwarePixelBufferSharedPtr buf = texturePtr->getBuffer();
		// TODO: Clear texture data.
		//Ogre::Image::Box tempBox(0,0,mTextureWidth-1,mTextureHeight-1);
		//buf->lock(tempBox,Ogre::HardwareBuffer::HBL_DISCARD);
		//buf->unlock();

		// Blit images to texture buffer
		// Minimum of 1 pixel between each image at all times
		for(size_t i = 0; i < mNumIndividualTextures; i++)
		{
			Ogre::Vector4 texCoord = mTextureMap[mTextureNames.at(i)];

			// Blit to the specified location on the texture
			buf->blitFromMemory(images.at(i).getPixelBox(),
				Ogre::Image::Box(texCoord.x * mTextureWidth, texCoord.y * mTextureHeight, 
								texCoord.z * mTextureWidth, texCoord.w * mTextureHeight));
		}

		// Following lines are unnecessary... just puts it out to a file to see :-)
/*		Ogre::Image finalImageSet;
		finalImageSet.load("QuickGUI.output.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		buf->blitToMemory(finalImageSet.getPixelBox());
		finalImageSet.save("SkinSetImage.png");
*/	
		mDirtyTexture = false;

		// save texture and script describing it
		saveSkin();
	}

	bool SkinSet::containsImage(Ogre::String textureName)
	{
		if(mContainedTextures.find(textureName) != mContainedTextures.end())
			return true;

		return false;
	}

	Ogre::String SkinSet::getImageExtension() const
	{
		return mImageExtension;
	}

	Ogre::String SkinSet::getSkinName() const
	{
		return mSkinName;
	}

	Ogre::String SkinSet::getTextureName() const
	{
		return mTextureName;
	}

	Ogre::Vector4 SkinSet::getTextureCoordinates(const Ogre::String &imageName) const
	{
		std::map<Ogre::String,Ogre::Vector4>::const_iterator itTexMap = mTextureMap.find(imageName);
		if (itTexMap == mTextureMap.end() )
			return Ogre::Vector4(0,0,1,1);
		return itTexMap->second;
	}

	void SkinSet::setTextureCoordinates(const Ogre::String &imageName, const Ogre::Vector4 &texCoord)
	{
		std::map<Ogre::String,Ogre::Vector4>::iterator itTexMap = mTextureMap.find(imageName);
		if (itTexMap == mTextureMap.end() )
			return;
		itTexMap->second = texCoord;
		mDirtyTexture = true;
	}
	void SkinSet::buildMaterial()
	{
		Ogre::MaterialPtr mp = Ogre::MaterialManager::getSingleton().create(mMaterialName, mResourceGroup);
		Ogre::Technique *t = mp->getTechnique(0);
		Ogre::Pass *p = t->getPass(0);

		p->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		p->setLightingEnabled(false);
		p->setCullingMode(Ogre::CULL_CLOCKWISE);

		Ogre::TextureUnitState *tu = p->createTextureUnitState(mTextureName);
		tu->setTextureName(mTextureName);
	}
}
