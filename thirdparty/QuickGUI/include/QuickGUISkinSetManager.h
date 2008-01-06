#ifndef QUICKGUISKINSETMANAGER_H
#define QUICKGUISKINSETMANAGER_H

#include "QuickGUIExportDLL.h"
#include "QuickGUISkinSet.h"

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"

#include <map>

namespace QuickGUI
{
	class _QuickGUIExport SkinSetManager :
		public Ogre::Singleton<SkinSetManager>
	{
	public:
		friend class Root;
	public:
		static SkinSetManager& getSingleton(void); 
		static SkinSetManager* getSingletonPtr(void);

		/**
		* Checks if skinName is a loaded skin SkinSet, and if textureName is an
		* embedded texture within the skin SkinSet.
		*/
		bool embeddedInSkinSet(const Ogre::String& skinName, const Ogre::String& textureName);
		bool embeddedInSkinSet(const Ogre::String& textureName);

		/**
		* Returns the SkinSet if exists, NULL otherwise.
		*/
		SkinSet* getSkinSet(const Ogre::String& name);
		/*
		* Returns skinset which has this texture name, null otherwise
		*/
		SkinSet* getSkinSetByTextureName(const Ogre::String& texName);
		std::map<Ogre::String,SkinSet*>* getSkinSetList();

		void loadSkin(const Ogre::String& skinName, SkinSet::ImageType t, const Ogre::String &resourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		
		/** Creates a skin, but does not immediately load it
		* @param skinName The name of the skin to use
		* @param t The image type to be used for extension
		* @param resourceGroup The name of the resource group to use
		* @note If a skinset of that name was already created, it will be returned instead of creating a new skinset
		* @note The caller is responsible for the initialization of the skinset (buildTexture call)
		* @note This version of the call enables caller to map image file names (use the SkinSet::setImageMapping for that)
		*/
		SkinSet* createSkin(const Ogre::String& skinName, SkinSet::ImageType t, const Ogre::String &resourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		/*
		* Returns true if a skin has been loaded with the name skinName, false otherwise.
		*/
		bool skinLoaded(const Ogre::String& skinName);

	protected:
		SkinSetManager();
		~SkinSetManager();

		std::map<Ogre::String,SkinSet*> mSkinSets;
		std::map<Ogre::String,SkinSet*> mSkinSetsbyTextureName;
	};
}

#endif
