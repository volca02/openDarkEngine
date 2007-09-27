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

#include "OIS.h"
#include "GameStateManager.h"
#include "GameLoadState.h"
#include "GamePlayState.h"

#include "GameService.h"

#include "logger.h"

#include <OgreOverlay.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreFontManager.h>

using namespace Ogre;

namespace Opde {

	template<> GameLoadState* Singleton<GameLoadState>::ms_Singleton = 0;

	GameLoadState::GameLoadState() : mSceneMgr(NULL), mOverlayMgr(NULL), mLoaded(false) {
        mRoot = Root::getSingletonPtr();
		mOverlayMgr = OverlayManager::getSingletonPtr();
		mServiceMgr = ServiceManager::getSingletonPtr();
		mConfigService = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();
	}

    GameLoadState::~GameLoadState() {

    }

	void GameLoadState::start() {
	    LOG_INFO("LoadState: Starting");

		mSceneMgr = mRoot->getSceneManager( "DarkSceneManager" );

		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);

		mCamera	= mSceneMgr->createCamera( "LoadCamera" );

		mViewport = mRoot->getAutoCreatedWindow()->addViewport( mCamera );

		// display loading... message
        mLoadingOverlay = OverlayManager::getSingleton().getByName("Opde/LoadOverlay");

		// Create a panel
		mLoadingOverlay->show();

		LOG_INFO("LoadState: Started");

		mLoaded = false;
		mFirstTime = true;
	}

	void GameLoadState::exit() {
	    LOG_INFO("LoadState: Exiting");
		// mLoadingOverlay->hide();

        mLoadingOverlay->hide();

		mSceneMgr->destroyAllCameras();

		mRoot->getAutoCreatedWindow()->removeAllViewports();

		LOG_INFO("LoadState: Exited");

		if (mLoaded) {
			pushState(GamePlayState::getSingletonPtr());
		}
	}

	void GameLoadState::suspend() {
	    LOG_INFO("LoadState: Suspend?!");
	}

	void GameLoadState::resume() {
	    LOG_INFO("LoadState: Resume?!");
	    mLoaded = false;
		mFirstTime = true;


        // mLoadingOverlay->show();
	}

	void GameLoadState::update(unsigned long timePassed) {
		if (!mFirstTime && !mLoaded) {
		    OverlayElement* guiLdr = OverlayManager::getSingleton().getOverlayElement("Opde/LoadPanel/Description");
		    guiLdr->setCaption("Loading, please wait...");

			mRoot->renderOneFrame();

			GameServicePtr gsvc = mServiceMgr->getService("GameService").as<GameService>();

      std::string misFile = mConfigService->getParam("mission");

			gsvc->load(misFile);

			mLoaded = true;

			guiLdr->setCaption("Loaded, press ESC...");

			popState(); // Hardcoded, so no escape key is needed
		}

		mFirstTime = false;
	}

	bool GameLoadState::keyPressed( const OIS::KeyEvent &e ) {
		return false;
	}

	bool GameLoadState::keyReleased( const OIS::KeyEvent &e ) {
		if( e.key == OIS::KC_ESCAPE ) {
        		// requestTermination();
			if (mLoaded)
				popState();
    		}
		return false;
	}

	bool GameLoadState::mouseMoved( const OIS::MouseEvent &e ) {
		return false;
	}

	bool GameLoadState::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		return false;
	}

	bool GameLoadState::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		return false;
	}

}

