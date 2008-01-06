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
#include "QuickGUISkinSet.h"
#include "QuickGUISkinSetManager.h"
#include "StringTokenizer.h"

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
			mRenderServiceListenerID(0),
			mSkinName("opde"),
			mSkinSet(NULL),
			mGUIMap("") {
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
		assert(mGUIManager);
		
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
		assert(mGUIManager);
		
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
		assert(mGUIManager);
		
		return mGUIManager->getActiveSheet();
	}
	
	// -----------------------------------
	void GUIService::setActiveSheet(QuickGUI::Sheet* sheet) {
		assert(mGUIManager);
		
		mGUIManager->setActiveSheet(sheet);
	}
	
	// -----------------------------------
	QuickGUI::Sheet* GUIService::createSheet() {
		assert(mGUIManager);
		
		return mGUIManager->createSheet();
	}
	
	// -----------------------------------
	void GUIService::destroySheet(QuickGUI::Sheet* sheet) {
		assert(mGUIManager);
		
		mGUIManager->destroySheet(sheet);
	}
	
	// -----------------------------------
	void GUIService::loadGUIMapping(const std::string& fname) {
		Ogre::DataStreamPtr dst = ResourceGroupManager::getSingleton().openResource(fname, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true, NULL);
			
		// For simplicity, we expect all the mapping to be for the skin specified in config file...
		int lineno = 0;
		
		// While there is another line left
		while (!dst->eof()) {
			lineno++;
			Ogre::String line = dst->getLine();
			
			// Process the line.
			// objectname[.*] 'imagename'
			// Quotes are optional, everything after .* before space is ignored
			WhitespaceStringTokenizer tok(line, false);
			
			// Probably an empty line
			if (tok.end())
				continue;
				
			
			
			String name = tok.next();
			
			if (name == "#") // comment
				continue;
				
				
			if (tok.end())	{
				LOG_ERROR("GUIService: Malformed mapping file '%s' line %d : %s", fname.c_str(), lineno, line.c_str());
				continue;
			}
				
			String filename = tok.next();
			
			bool subscope = false;
			
			// See if there are wildcards or not.
			size_t wildpos = name.find(".*");
			
			if (wildpos != String::npos) {
				// Strip the string of the wildcard
				subscope = true;
				
				name = name.substr(0, wildpos);
			}
			
			// Map to the skinset
			mSkinSet->setImageMapping(name, filename, subscope);
		}
	}

	// -----------------------------------
	bool GUIService::init() {
		// Initialize the QuickGUI for the active viewport
		
		// Nothing to do, we have to wait for RenderService to settle in init... Better do everything in bootstrapFinished thus
		// One thing happens here though - loading of the mapping file based on a config file value
		ConfigServicePtr cfp = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();
		
		// if we have a config value (and the specified file is loadable), load the mapping file
		DVariant val;
		
		if (cfp->getParam("gui_skin_name", val)) {
			mSkinName = val.toString();
			
			if (mSkinName == "") {
				LOG_ERROR("GUIService: Empty skin name, defaulting to 'opde'");
				mSkinName = "opde";
			}
		}
		
		if (cfp->getParam("gui_mapping", val)) {
			// Try to open a file, load config
			mGUIMap = val.toString();
		} else {
			mGUIMap = "";
		}
		
		
		return true;
	}
	
	// -----------------------------------
	void GUIService::bootstrapFinished() {
		mInputSrv = ServiceManager::getSingleton().getService("InputService").as<InputService>();
		mRenderSrv = ServiceManager::getSingleton().getService("RenderService").as<RenderService>();
		
		mRoot = new QuickGUI::Root();
		
		QuickGUI::registerScriptParser();

		mSkinSet = QuickGUI::SkinSetManager::getSingleton().createSkin(mSkinName, QuickGUI::SkinSet::IMAGE_TYPE_PNG);

		assert(mSkinSet);
		
		if (mGUIMap != "")
			loadGUIMapping(mGUIMap);
		
		//		if (!mSkinSet->loadSkin())
		mSkinSet->buildTexture();

        assert(mRenderSrv->getSceneManager());
        
   		mGUIManager = mRoot->createGUIManager(mRenderSrv->getDefaultViewport());
        mGUIManager->setSceneManager(mRenderSrv->getSceneManager());
        
		// handler for direct listener
		mInputSrv->setDirectListener(this);
		
		// Register as a listener for the resolution changes
		RenderService::ListenerPtr renderServiceListener = new ClassCallback<RenderServiceMsg, GUIService>(this, &GUIService::onRenderServiceMsg);
		mRenderServiceListenerID = mRenderSrv->registerListener(renderServiceListener);
		
		// Enable mouse pointer
		mGUIManager->getMouseCursor()->show();
		
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

    GUIServiceFactory::GUIServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
    };

    const std::string& GUIServiceFactory::getName() {
		return mName;
    }

    Service* GUIServiceFactory::createInstance(ServiceManager* manager) {
		return new GUIService(manager, mName);
    }

}
