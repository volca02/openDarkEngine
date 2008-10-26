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
 *
 *
 *    $Id$
 *
 *****************************************************************************/

#include "config.h"

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
        mRoot = Ogre::Root::getSingletonPtr();
		mOverlayMgr = OverlayManager::getSingletonPtr();
		mServiceMgr = ServiceManager::getSingletonPtr();
		
		mConfigService = static_pointer_cast<ConfigService>(ServiceManager::getSingleton().getService("ConfigService"));

		mFontTest = false;

		mManualFonLoader = new ManualFonFileLoader();

		// T1 only. Does not matter much though. This is only a sort of unit test. Will be removed
		mFontList.push_back("object/textfont.fon");
		mFontList.push_back("objectlo/textfont.fon");
		mFontList.push_back("objectlou/textfont.fon");
		mFontList.push_back("OBJECTU/textfont.fon");
		mFontList.push_back("ledger2/TEXTFONT.FON");
		mFontList.push_back("demo/TEXTFONT.FON");
		mFontList.push_back("parch/TEXTFONT.FON");
		mFontList.push_back("parch2/TEXTFONT.FON");
		mFontList.push_back("STONED4/TEXTFONT.FON");
		mFontList.push_back("mapscrap/TEXTFONT.FON"); // TODO:  Shows up as totally transparent
		mFontList.push_back("graveD10/TEXTFONT.FON");
		mFontList.push_back("PBOOK/TEXTFONT.FON");
		mFontList.push_back("plaque/TEXTFONT.FON");
		mFontList.push_back("pbook2/TEXTFONT.FON");
		mFontList.push_back("keepmap/TEXTFONT.FON"); // TODO:  Shows up as totally transparent
		mFontList.push_back("parch3/TEXTFONT.FON");
		mFontList.push_back("ledger/TEXTFONT.FON");

		// mFontList.push_back("font.fon"); // Monochromatic, always ok
		// mFontList.push_back("textfont.fon");  // Monochromatic, always ok

		mFontList.push_back("FONTAA36.FON");
		// mFontList.push_back("SMALFONT.FON");  // Monochromatic, always ok
		mFontList.push_back("FONTAA29.FON");
		// mFontList.push_back("TEXTFONT.FON");  // Monochromatic, always ok
		mFontList.push_back("FONTAA20.FON");
		mFontList.push_back("FONTAA16.FON");
		mFontList.push_back("FONTAA12.FON");
	}

    GameLoadState::~GameLoadState() {
    	delete mManualFonLoader;
    }

	void GameLoadState::start() {
		DVariant fnttst;

		if (mConfigService->getParam("font_test", fnttst))
			mFontTest = fnttst.toBool();

	    LOG_INFO("LoadState: Starting");

		mSceneMgr = mRoot->getSceneManager( "DarkSceneManager" );

		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);

		RenderServicePtr renderSrv = static_pointer_cast<RenderService>(ServiceManager::getSingleton().getService("RenderService"));
		
		mCamera = renderSrv->getDefaultCamera();
		mViewport = renderSrv->getDefaultViewport();

		if (mFontTest) {
			// create all the fonts in T1 as a test
			// You may have some trouble here - I searched for all the .fon files in probably all .crf files of T1
			// So be sure to fill your resources.cfg to reflect this is you wanna use this code (font_test=true in opde.cfg)
			// Anyway, this is the list
			createTestFontOverlays();
		} else {
			// display loading... message
			mLoadingOverlay = OverlayManager::getSingleton().getByName("Opde/LoadOverlay");

			// Create a panel
			mLoadingOverlay->show();
		}

		LOG_INFO("LoadState: Started");

		mLoaded = false;
		mFirstTime = true;
	}

	void GameLoadState::exit() {
	    LOG_INFO("LoadState: Exiting");
		// mLoadingOverlay->hide();

        mLoadingOverlay->hide();

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
			unsigned long start_ms = Ogre::Root::getSingleton().getTimer()->getMilliseconds();
			
		    OverlayElement* guiLdr = OverlayManager::getSingleton().getOverlayElement("Opde/LoadPanel/Description");
		    guiLdr->setCaption("Loading, please wait...");

			mRoot->renderOneFrame();

			GameServicePtr gsvc = static_pointer_cast<GameService>(mServiceMgr->getService("GameService"));

			std::string misFile = mConfigService->getParam("mission");

			gsvc->load(misFile);

			mLoaded = true;

			guiLdr->setCaption("Loaded, press ESC...");
			
			LOG_INFO("Loading took %10.2f seconds", (Ogre::Root::getSingleton().getTimer()->getMilliseconds() - start_ms) / 1000.0f);

			// popState(); // Hardcoded, so no escape key is needed
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

	void GameLoadState::createTestFontOverlays() {
		// Create a red material as a background
		MaterialPtr material = MaterialManager::getSingleton().create(
			"Colour/Red", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		/*material->getTechnique(0)->getPass(0)->setAmbient(1,0,0);
		material->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,0);
		material->getTechnique(0)->getPass(0)->setSpecular(1,0,0,0);*/
		material->getTechnique(0)->getPass(0)->createTextureUnitState();
		material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setColourOperationEx(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, ColourValue(0.58, 0.56, 0.35));

		// Iterate through the list, create an overlay for each font
		OverlayManager& overlayManager = OverlayManager::getSingleton();

		mLoadingOverlay = overlayManager.create("TestFonts");

		// Create a panel
		OverlayContainer* panel = static_cast<OverlayContainer*>(
			overlayManager.createOverlayElement("Panel", "PanelName"));

		panel->setMetricsMode(Ogre::GMM_RELATIVE);
		panel->setPosition(0, 0);
		panel->setDimensions(1, 1); // Full screen
		
		// TODO: Background material?
		panel->setMaterialName("Colour/Red");


		std::vector<std::string>::iterator it = mFontList.begin();
		int posy = 0; int height = 16;

		for (; it != mFontList.end(); it++) {
			// Create a text area
			TextAreaOverlayElement* textArea = static_cast<TextAreaOverlayElement*>(
				overlayManager.createOverlayElement("TextArea", *it));

			Ogre::LogManager::getSingleton().logMessage("Loading font test " + *it);

			String text = "Keepers would like this font: " + *it+ ", LOADING!";
			
			Ogre::FontPtr fnt = FontManager::getSingleton().create(*it, "General");
			
			// A test. For parch book load a custom font palette
			if ((*it == "FONTAA36.FON") || (*it == "FONTAA29.FON")) {
				mManualFonLoader->setPalette(ManualFonFileLoader::ePT_PCX, "BOOK.PCX");
				LOG_INFO("Loading a custom palette for the font %s", (*it).c_str());
				text = "Garrett would like pcx palette font: " + *it+ ", LOADING!";
			} else {
				mManualFonLoader->setPalette();
			}
			
			
			mManualFonLoader->loadResource(&(*fnt));

			height = StringConverter::parseInt(fnt->getParameter("size"));

			textArea->setMetricsMode(Ogre::GMM_PIXELS);
			textArea->setPosition(0, posy);
			textArea->setDimensions(640, 45);

			// textArea->setCaption(*it);
			textArea->setCaption(text);

            // height = 16;
			textArea->setCharHeight(height); // Todo: size
			textArea->setFontName(fnt->getName()); // *it

			textArea->setColour(ColourValue(1, 1, 1));
			// textArea->setColourTop(ColourValue(0.5, 0.7, 0.5));

			posy += height + 4;

			// Add the text area to the panel
			panel->addChild(textArea);
		}

		mLoadingOverlay->add2D(panel);

		// Show the overlay
		mLoadingOverlay->show();
	}

	GameLoadState& GameLoadState::getSingleton() {
		assert(ms_Singleton); return *ms_Singleton;
	}

	GameLoadState* GameLoadState::getSingletonPtr() {
		return ms_Singleton;
	}
}

