#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIRoot.h"

template<> QuickGUI::Root* Ogre::Singleton<QuickGUI::Root>::ms_Singleton = 0; 

namespace QuickGUI
{
	Root::Root() :
		mGUIManagerCounter(-1)
	{
		// Create SkinSetManager
		new SkinSetManager();
	}

	Root::~Root()
	{
		for(std::map<Ogre::String,GUIManager*>::iterator it = mGUIManagers.begin(); it != mGUIManagers.end(); ++it)
			delete (*it).second;
		mGUIManagers.clear();

		// Destroy SkinSetManager
		delete SkinSetManager::getSingletonPtr();
	}

	Root* Root::getSingletonPtr(void) 
	{ 
		return ms_Singleton; 
	}

	Root& Root::getSingleton(void) 
	{ 
		assert( ms_Singleton );  
		return ( *ms_Singleton ); 
	}

	GUIManager* Root::createGUIManager(const Ogre::String& name, Ogre::Viewport* v)
	{
		// make sure name does not already exist.
		if(mGUIManagers.find(name) != mGUIManagers.end())
			throw Ogre::Exception(Ogre::Exception::ERR_DUPLICATE_ITEM,"A GUIManager with name \"" + name + "\" already exists!","Root::createGUIManager");

		GUIManager* newGUIManager = new GUIManager(name,v);
		mGUIManagers[name] = newGUIManager;

		return newGUIManager;
	}

	GUIManager* Root::createGUIManager(Ogre::Viewport* v)
	{
		++mGUIManagerCounter;
		return createGUIManager("GUIManager" + Ogre::StringConverter::toString(mGUIManagerCounter),v);
	}

	void Root::destroyGUIManager(GUIManager* gm)
	{
		if(gm == NULL)
			return;

		destroyGUIManager(gm->getName());
	}

	void Root::destroyGUIManager(const Ogre::String& name)
	{
		GUIManager* gm = mGUIManagers[name];
		mGUIManagers.erase(mGUIManagers.find(name));
		delete gm;
	}

	GUIManager* Root::getGUIManager(const Ogre::String& name)
	{
		if(mGUIManagers.find(name) == mGUIManagers.end())
			return NULL;

		return mGUIManagers[name];
	}
}
