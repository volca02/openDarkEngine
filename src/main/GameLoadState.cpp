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
	}
	
	void GameLoadState::start() {
		mRoot = Root::getSingletonPtr();
		mOverlayMgr = OverlayManager::getSingletonPtr();
		mSceneMgr = mRoot->getSceneManager( "DarkSceneManager" );
		
		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);
		
		mCamera	= mSceneMgr->createCamera( "MainCamera" );
		
		mViewport = mRoot->getAutoCreatedWindow()->addViewport( mCamera );
		
		mServiceMgr = ServiceManager::getSingletonPtr();
		
		// display loading... message
		
		// Create a panel
		mLoadingOverlay = OverlayManager::getSingleton().getByName("Opde/LoadOverlay");
		mLoadingOverlay->show();
		
		LOG_INFO("LoadState: Started");
		
		mLoaded = false;
		mFirstTime = true;
	}
	
	void GameLoadState::exit() {
		mLoadingOverlay->hide();
		
		mSceneMgr->destroyAllCameras();
		
		mRoot->getAutoCreatedWindow()->removeAllViewports();
		
		LOG_INFO("LoadState: Exited");
		
		if (mLoaded) {
			GamePlayState* st = new GamePlayState();
			pushState(st);
			st->release();
		}
	}
	
	void GameLoadState::suspend() {
	}
	
	void GameLoadState::resume() {
	}
	
	void GameLoadState::update(unsigned long timePassed) {
		if (!mFirstTime && !mLoaded) {
			
			mRoot->renderOneFrame();
			
			GameServicePtr gsvc = mServiceMgr->getService("GameService").as<GameService>();
			
			gsvc->load("miss1.mis");
		
			mLoaded = true;
			OverlayElement* guiLdr = OverlayManager::getSingleton().getOverlayElement("Opde/LoadPanel/Description");
			
			guiLdr->setCaption("Loaded, press ESC...");
		}
		
		mFirstTime = false;
	}

	bool GameLoadState::keyPressed( const OIS::KeyEvent &e ) {
	}
	
	bool GameLoadState::keyReleased( const OIS::KeyEvent &e ) {
		if( e.key == OIS::KC_ESCAPE ) {
        		// requestTermination();
			if (mLoaded)
				popState();
    		}
	}
	
	bool GameLoadState::mouseMoved( const OIS::MouseEvent &e ) {
	}
	
	bool GameLoadState::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
	}
	
	bool GameLoadState::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
	}
	
}

