/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/
 
#include "OpdeGUI.h"
#include "Ogre.h"
#include "OpdeApplication.h"
#include "OpdeLoadingBar.h"
#include "OpdeMission.h"
#include "OgreDarkSceneManager.h"

// For now... Will be in this namespace too...
using namespace Opde;

/** main class. For testing purposes now. */
class OpdeApplication : public ExampleApplication {
	private:
		OpdeGUI	*gui;
		OpdeMission *mission;
		DarkSceneManagerFactory* darkFactory;
	public:
		OpdeApplication() {

		}

		~OpdeApplication() {
			if (gui) {
				delete gui;
				gui = NULL;
			}
			
			if (mission) {
				delete mission;
				mission = NULL;
			}
			
			Root::getSingleton().removeSceneManagerFactory(darkFactory);
			delete darkFactory;
		}
		
	protected:
		String mMission;
		
		ExampleLoadingBar mLoadingBar;

		void loadResources(void) {
			// TODO: Hacky. But this all will go away anyway
			mission = new OpdeMission(mRoot);
			
			mLoadingBar.start(mWindow, 1, 1, 0.75);

			// Turn off rendering of everything except overlays
			mSceneMgr->clearSpecialCaseRenderQueues();
			mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
			mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);

			// Set up the world geometry link
			/*ResourceGroupManager::getSingleton().linkWorldGeometryToResourceGroup(
				ResourceGroupManager::getSingleton().getWorldResourceGroupName(), 
				mMission, mSceneMgr);
			*/
			// Initialise the rest of the resource groups, parse scripts etc
			ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
			
			ResourceGroupManager::getSingleton().loadResourceGroup(
				ResourceGroupManager::getSingleton().getWorldResourceGroupName(),
				false, true);
			
			mission->loadMission(mMission);
			
			// Back to full rendering
			mSceneMgr->clearSpecialCaseRenderQueues();
			mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);

			mLoadingBar.finish();
		}

		// Override resource sources (include Quake3 archives)
		void setupResources(void) {
			// Load Quake3 locations from a file
			ConfigFile cf;

			cf.load("opde.cfg");

			mMission = cf.getSetting("Map");

			ExampleApplication::setupResources();

			/*ResourceGroupManager::getSingleton().addResourceLocation(
				mQuakeLevel, "", ResourceGroupManager::getSingleton().getWorldResourceGroupName());*/
		}
		
		// Override scene manager (use indoor instead of generic)
		void chooseSceneManager(void) {
			// Create new scene manager
			darkFactory = new DarkSceneManagerFactory();

			// Register
			Root::getSingleton().addSceneManagerFactory(darkFactory);
			
			mSceneMgr = mRoot->createSceneManager(ST_INTERIOR, "DarkSceneManager"); // Sure we have to create our scenemanager first!
			mSceneMgr = mRoot->getSceneManager("DarkSceneManager");
		}
		
		// Scene creation
		void createScene(void) {
			// modify camera for close work
			mCamera->setNearClipDistance(0.5);
			mCamera->setFarClipDistance(4000);

			// Also change position, and set Quake-type orientation
			// Get random player start point
			ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
			mCamera->setPosition(vp.position);
			mCamera->pitch(Degree(90)); // Quake uses X/Y horizon, Z up
			mCamera->rotate(vp.orientation);
			
			// Don't yaw along variable axis, causes leaning
			mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);
			
				    
			// Thiefy FOV
			mCamera->setFOVy(Degree(70));
		}
		
		void createFrameListener(void) {
			mFrameListener = new ExampleFrameListener(mWindow, mCamera);
			mFrameListener->showDebugOverlay(true);
			mRoot->addFrameListener(mFrameListener);
			
			gui = new OpdeGUI(mWindow, mCamera, mFrameListener);
			gui->setup(mSceneMgr);
			
			mRoot->addFrameListener(gui);
			
			// TODO: A hack. FrameListener for the camera and others should use the event system (E.g. buffered input)
			mFrameListener->setInputReader(gui->getInputReader());
		}
		
		bool handleQuit(const CEGUI::EventArgs& e) {
			gui->handleQuit(e);
			return true;
		}
};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char**argv)
#endif
{
    // Create application object
    OpdeApplication app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << 
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}
