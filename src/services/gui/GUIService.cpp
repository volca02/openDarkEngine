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

#include "ServiceCommon.h"
#include "GUIService.h"

using namespace std;
using namespace Ogre;
using namespace OIS;

namespace Opde {

    /*-----------------------------------------------------*/
    /*-------------------- InputService -------------------*/
    /*-----------------------------------------------------*/
    GUIService::GUIService(ServiceManager *manager, const std::string& name) : Service(manager, name), 
			mInputSrv(NULL),
			mRenderSrv(NULL),
			mGUIManager(NULL),
			mActive(false),
			mVisible(false),
			mRenderServiceListenerID(0) {
    }
    
    // -----------------------------------
    GUIService::~GUIService() {
    	if (!mInputSrv.isNull()) {
    		mInputSrv->unsetDirectListener();
    	}
    	
    	// Destroy the GUI manager
    	mRoot->destroyGUIManager(mGUIManager);
    	delete mRoot;
    	
    	if (!mRenderSrv.isNull())
			mRenderSrv->unregisterListener(mRenderServiceListenerID);
    }

	// -----------------------------------
	void GUIService::setActive(bool active) {
		/* What do we do here?
			if set to true, we set direct input mode and show cursor
			if set to false, we set mapped input mode and hide the cursor
		*/
		assert(!mInputSrv.isNull());
		assert(!mGUIManager);
		
		if (active) {
			mInputSrv->setInputMode(IM_DIRECT);
			mGUIManager->getMouseCursor()->show();
		} else {
			mInputSrv->setInputMode(IM_MAPPED);
			mGUIManager->getMouseCursor()->hide();
		}
		
		mActive = active;
	}
	
	// -----------------------------------
	void GUIService::setVisible(bool visible) {
		assert(!mGUIManager);
		
		QuickGUI::Sheet* sheet;
		sheet =	mGUIManager->getActiveSheet();
		
		if (visible) {
			if (sheet)
				sheet->show();
		} else {
			if (sheet)
				sheet->hide();
		}
		
		mVisible = visible;
	}
	
	// -----------------------------------
	QuickGUI::Sheet* GUIService::getActiveSheet() {
		assert(!mGUIManager);
		
		return mGUIManager->getActiveSheet();
	}
	
	// -----------------------------------
	void GUIService::setActiveSheet(QuickGUI::Sheet* sheet) {
		assert(!mGUIManager);
		
		mGUIManager->setActiveSheet(sheet);
	}
	
	// -----------------------------------
	QuickGUI::Sheet* GUIService::createSheet() {
		assert(!mGUIManager);
		
		return mGUIManager->createSheet();
	}
	
	// -----------------------------------
	void GUIService::destroySheet(QuickGUI::Sheet* sheet) {
		assert(!mGUIManager);
		
		mGUIManager->destroySheet(sheet);
	}

	// -----------------------------------
	bool GUIService::init() {
		// Nothing to do, we have to wait for RenderService to settle in init... Better do everything in bootstrapFinished thus
		return true;
	}
	
	// -----------------------------------
	void GUIService::bootstrapFinished() {
		mInputSrv = ServiceManager::getSingleton().getService("InputService").as<InputService>();
		mRenderSrv = ServiceManager::getSingleton().getService("RenderService").as<RenderService>();
		
		// Initialize the QuickGUI for the active viewport
		mRoot = new QuickGUI::Root();
		
        mGUIManager = mRoot->createGUIManager(mRenderSrv->getDefaultViewport());
        mGUIManager->setSceneManager(mRenderSrv->getSceneManager());
        
		// handler for direct listener
		mInputSrv->setDirectListener(this);
		
		// Register as a listener for the resolution changes
		RenderService::ListenerPtr renderServiceListener = new ClassCallback<RenderServiceMsg, GUIService>(this, &GUIService::onRenderServiceMsg);
		mRenderServiceListenerID = mRenderSrv->registerListener(renderServiceListener);
	}


	// -----------------------------------
	bool GUIService::keyPressed( const OIS::KeyEvent &e ) {
		mGUIManager->injectKeyDown(static_cast<QuickGUI::KeyCode>(e.key));
        mGUIManager->injectChar( e.text );
        return true;
	}
	
	// -----------------------------------
	bool GUIService::keyReleased( const OIS::KeyEvent &e ) {
		mGUIManager->injectKeyUp(static_cast<QuickGUI::KeyCode>(e.key));
		return true;
	}

	// -----------------------------------
	bool GUIService::mouseMoved( const OIS::MouseEvent &e ) {
		mGUIManager->injectMouseMove( e.state.X.rel, e.state.Y.rel );
		return true;
	}
	
	// -----------------------------------
	bool GUIService::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		mGUIManager->injectMouseButtonDown(static_cast<QuickGUI::MouseButtonID>(id));
		return true;
	}
	
	// -----------------------------------
	bool GUIService::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		mGUIManager->injectMouseButtonUp(static_cast<QuickGUI::MouseButtonID>(id));
		return true;
	}
	
	// -----------------------------------
	void GUIService::onRenderServiceMsg(const RenderServiceMsg& message) {
		// TODO: Inform the manager about the resolution change
	}
	
    //-------------------------- Factory implementation
    std::string GUIServiceFactory::mName = "GUIService";

    InputServiceFactory::InputServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
    };

    const std::string& InputServiceFactory::getName() {
		return mName;
    }

    Service* InputServiceFactory::createInstance(ServiceManager* manager) {
		return new GUIService(manager, mName);
    }

}
