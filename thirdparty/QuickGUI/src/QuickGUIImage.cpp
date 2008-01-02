#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIImage.h"
#include "QuickGUIManager.h"
#include "QuickGUIUtility.h" 

namespace QuickGUI
{
	Image::Image(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mMaterialName("")
	{
		mWidgetType = TYPE_IMAGE;
		mSkinComponent = ".image";
		mSize = Size(50,50);
	}

	Image::~Image()
	{
	}

	Ogre::String Image::getMaterialName()
	{
		return mMaterialName;
	}

	void Image::setMaterial(const Ogre::String& materialName)
	{
		if(mTextureLocked)
			return;

		mMaterialName = materialName;
		mQuad->setMaterial(mMaterialName);
		mQuad->setTextureCoordinates(Ogre::Vector4(0,0,1,1));

		if(Ogre::MaterialManager::getSingleton().resourceExists(materialName))
		{
			// make sure the material is loaded.
			Ogre::MaterialPtr mp = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().getByName(materialName));
			mp->load();

			Ogre::Pass* p = mp->getBestTechnique(0)->getPass(0);
			if(p->getNumTextureUnitStates() > 0)
			{
				Ogre::String textureName = p->getTextureUnitState(0)->getTextureName();

				if(Utility::textureExistsOnDisk(textureName))
				{
					Ogre::Image i;
					i.load(textureName,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
					delete mWidgetImage;
					mWidgetImage = new Ogre::Image(i);
				}
			}
		}
	}
}
