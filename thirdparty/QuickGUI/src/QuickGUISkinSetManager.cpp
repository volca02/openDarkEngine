#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIPrerequisites.h"

#include "QuickGUISkinSetManager.h"

template<> QuickGUI::SkinSetManager* Ogre::Singleton<QuickGUI::SkinSetManager>::ms_Singleton = 0; 

namespace QuickGUI
{
	SkinSetManager::SkinSetManager()
	{
	}

	SkinSetManager::~SkinSetManager()
	{
		// delete imagesets
		std::map<Ogre::String,SkinSet*>::iterator it;
		for( it = mSkinSets.begin(); it != mSkinSets.end(); ++it )
			delete (it->second);
		mSkinSets.clear();
		mSkinSetsbyTextureName.clear();
	}

	SkinSetManager* SkinSetManager::getSingletonPtr(void) { return ms_Singleton; } 
	SkinSetManager& SkinSetManager::getSingleton(void) { assert( ms_Singleton );  return ( *ms_Singleton ); } 

	bool SkinSetManager::embeddedInSkinSet(const Ogre::String& skinName, const Ogre::String& textureName)
	{
		if(!skinLoaded(skinName)) 
			return false;
		else 
			return mSkinSets[skinName]->containsImage(textureName);
	}

	bool SkinSetManager::embeddedInSkinSet(const Ogre::String& textureName)
	{
		std::map<Ogre::String,SkinSet*>::iterator it;
		for( it = mSkinSets.begin(); it != mSkinSets.end(); ++it )
			if( it->second->containsImage(textureName) )
				return true;

		return false;
	}

	SkinSet* SkinSetManager::getSkinSet(const Ogre::String& name)
	{
		if(!skinLoaded(name)) 
			return NULL;
		else 
			return mSkinSets[name];
	}

	SkinSet* SkinSetManager::getSkinSetByTextureName(const Ogre::String& texName)
	{
		std::map<Ogre::String,SkinSet*>::iterator skinSetIt = mSkinSetsbyTextureName.find(texName);
		if (skinSetIt != mSkinSetsbyTextureName.end())
			return skinSetIt->second;
		return NULL;
	}

	std::map<Ogre::String,SkinSet*>* SkinSetManager::getSkinSetList()
	{
		return &mSkinSets;
	}

	void SkinSetManager::loadSkin(const Ogre::String& skinName, SkinSet::ImageType t, const Ogre::String &resourceGroup)
	{
		// check if imageset is already created for this skin
		if( mSkinSets.find(skinName) != mSkinSets.end() )
			return;

		SkinSet *skinSet = new SkinSet(skinName, t, resourceGroup);
		mSkinSets[skinName] = skinSet;
		mSkinSetsbyTextureName[skinSet->getTextureName()] = skinSet;
	}
	
	SkinSet* SkinSetManager::createSkin(const Ogre::String& skinName, SkinSet::ImageType t, const Ogre::String &resourceGroup)
	{
		std::map<Ogre::String,SkinSet*>::const_iterator it = mSkinSets.find(skinName);
		
		// check if imageset is already created for this skin
		if( it != mSkinSets.end() )
			return it->second;

		SkinSet *skinSet = new SkinSet(skinName, t, resourceGroup, false);
		
		mSkinSets[skinName] = skinSet;
		mSkinSetsbyTextureName[skinSet->getTextureName()] = skinSet;
		
		return skinSet;
	}

	bool SkinSetManager::skinLoaded(const Ogre::String& skinName)
	{
		return (mSkinSets.find(skinName) != mSkinSets.end());
	}
}
