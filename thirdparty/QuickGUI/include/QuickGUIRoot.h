#ifndef QUICKGUIROOT_H
#define QUICKGUIROOT_H

#include "QuickGUIExportDLL.h"
#include "QuickGUIManager.h"
#include "QuickGUISkinSetManager.h"

#include "OgreException.h"
#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreViewport.h"

#include <map>

namespace QuickGUI
{
	class _QuickGUIExport Root :
		public Ogre::Singleton<Root>
	{
	public:
		Root();
		~Root();

		static Root& getSingleton(void); 
		static Root* getSingletonPtr(void);

		GUIManager* createGUIManager(const Ogre::String& name, Ogre::Viewport* v);
		GUIManager* createGUIManager(Ogre::Viewport* v);

		void destroyGUIManager(GUIManager* gm);
		void destroyGUIManager(const Ogre::String& name);

		GUIManager* getGUIManager(const Ogre::String& name);

	protected:
		int mGUIManagerCounter;

		std::map<Ogre::String,GUIManager*> mGUIManagers;
	};
}

#endif
