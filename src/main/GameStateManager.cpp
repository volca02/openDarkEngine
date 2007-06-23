/******************************************************************************
*
*    This file is part of openDarkEngine project
*    Copyright (C) 2005-2006 openDarkEngine team
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
* 
* You should have received a copy of the GNU Lesser General Public License along with
* this program; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place - Suite 330, Boston, MA 02111-1307, USA, or go to
* http://www.gnu.org/copyleft/lesser.txt.
*****************************************************************************/

#include "GameStateManager.h"
#include "OpdeException.h"
#include "logger.h"
#include "stdlog.h"

// All the services
#include "WorldRepService.h"
#include "BinaryService.h"
#include "GameService.h"
#include "LinkService.h"

#include <OgreRoot.h>
#include <OgreWindowEventUtilities.h>
#include <OgreConfigFile.h>

using namespace Ogre;

namespace Opde {
	
	// The instance owner	
	template<> GameStateManager* Singleton<GameStateManager>::ms_Singleton = 0;

	GameStateManager::GameStateManager() : 
			mStateStack(), 
			mTerminate(false), 
			mRoot(NULL),
			mDarkSMFactory(NULL),
			mLogger(NULL),
			mStdLog(NULL),
			mConsoleBackend(NULL),
			mInputSystem(NULL),
			mRenderWindow(NULL),
			mServiceMgr(NULL),
			mDTypeScriptLdr(NULL) {
		
	}
	
	GameStateManager::~GameStateManager() {
		while (!mStateStack.empty()) {
			GameState* state = mStateStack.top();
			mStateStack.pop();
			state->exit();
		}
		
		if (mDarkSMFactory) {
			Root::getSingleton().removeSceneManagerFactory(mDarkSMFactory);
			delete mDarkSMFactory;
			mDarkSMFactory = NULL;
		}
		
		delete mDTypeScriptLdr; // Unregisters itself
		mDTypeScriptLdr = NULL;
		
		delete mPLDefScriptLdr; // Unregisters itself
		mPLDefScriptLdr = NULL;
		
		// release the mouse and keyboard, then the whole inputsystem
		if (mInputSystem) {
			if( mMouse ) {
				mInputSystem->destroyInputObject( mMouse );
				mMouse = NULL;
			}
	
			if( mKeyboard ) {
				mInputSystem->destroyInputObject( mKeyboard );
				mKeyboard = NULL;
			}
			
			mInputSystem->destroyInputSystem(mInputSystem);
			mInputSystem = NULL;
		}
		
		// Delete the service manager
		delete mServiceMgr;
		mServiceMgr = NULL;
		
		// Releas
		delete mLogger;
		delete mStdLog;
		
		delete mConsoleBackend;
		delete mRoot;
	}
	
	GameStateManager& GameStateManager::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );
	}
		
	GameStateManager* GameStateManager::getSingletonPtr(void) {
		return ms_Singleton;
	}
	
	//------------------ Main implementation ------------------
	void GameStateManager::terminate() {
		mTerminate = true;
	}
	
	void GameStateManager::pushState(GameState* state) {
		if (!mStateStack.empty()) {
			mStateStack.top()->suspend();
		}
		
		state->addRef();
		mStateStack.push(state);
		
		state->start();
	}
	
	void GameStateManager::popState() {
		if (!mStateStack.empty()) {
			GameState* old = mStateStack.top();
			
			mStateStack.pop();
			
			old->exit();
			
			old->release();
			
			if (!mStateStack.empty()) { // There is another state in the stack
				mStateStack.top()->resume();
			} else {
				// mTerminate = true; // Terminate automatically if this state was the last?
			}
		} else {
			OPDE_EXCEPT("State stack was empty, nothing could be done.","StateManager::popState");
		}
	}

	
	bool GameStateManager::run(GameState* state) {
		// Initialize opde logger and console backend
		mLogger = new Logger();
			
		mStdLog = new StdLog();
		
		mLogger->registerLogListener(mStdLog);
		
		// Create an ogre's root
		mRoot = new Root();
		
		mConsoleBackend = new Opde::ConsoleBackend();
		
		// So console will write the messages from the logger
		mLogger->registerLogListener(mConsoleBackend);
		
		mConsoleBackend->putMessage("==Console Starting==");
		
		
		// Create the service manager
		mServiceMgr = new ServiceManager();
		
		// Register the worldrep service factory
		registerServiceFactories();

		// Allocate and register the DType script loader. Registers itself
		mDTypeScriptLdr = new DTypeScriptLoader();
		
		// Allocate and register the PLDef script loader. Registers itself
		mPLDefScriptLdr = new PLDefScriptLoader();
		
		// Setup resources.
		setupResources();
	
		if (!configure()) 
			return false;
	
		// Set default mipmap level (NB some APIs ignore this)
		TextureManager::getSingleton().setDefaultNumMipmaps(5);
	
		// Initialize the input system
		setupInputSystem(); // must be after configure. as configure creates the window
		
		// Push the initial state
		pushState(state);
		
		// Run the game loop
		// Main while-loop
		while( !mTerminate ) {
			// Calculate time since last frame and remember current time for next frame
			unsigned long lTimeCurrentFrame = mRoot->getTimer()->getMilliseconds();
			unsigned long lTimeSinceLastFrame = lTimeCurrentFrame - mTimeLastFrame;
			mTimeLastFrame = lTimeCurrentFrame;
		
			// Update inputmanager
			captureInputs();
		
			// Update current state
			mStateStack.top()->update( lTimeSinceLastFrame );
		
			// Render next frame
			mRoot->renderOneFrame();
		
			// Deal with platform specific issues
			Ogre::WindowEventUtilities::messagePump();
		}

		return true;
	}
	
	void GameStateManager::registerServiceFactories() {
		// register the worldrep factory
		new WorldRepServiceFactory();
		new BinaryServiceFactory();
		new GameServiceFactory();
		new LinkServiceFactory();
	}
			
	/// Method which will define the source of resources (other than current folder)
	void GameStateManager::setupResources(void) {
		// Load resource paths from config file
		ConfigFile cf;
		cf.load("resources.cfg");

		// Go through all sections & settings in the file
		ConfigFile::SectionIterator seci = cf.getSectionIterator();

		String secName, typeName, archName;
		
		while (seci.hasMoreElements()) {
			secName = seci.peekNextKey();
			ConfigFile::SettingsMultiMap *settings = seci.getNext();
			ConfigFile::SettingsMultiMap::iterator i;
			
			for (i = settings->begin(); i != settings->end(); ++i) {
				typeName = i->first;
				archName = i->second;
				ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
			}
		}
	}
	
	bool GameStateManager::configure() {
		// Copied from the OIS Example. Thanks
		// Load config settings from ogre.cfg
    		if( !mRoot->restoreConfig() ) {
        		// If there is no config file, show the configuration dialog
        		if( !mRoot->showConfigDialog() )
            			return false;
    		}

		// Initialise and create a default rendering window
		mRenderWindow = mRoot->initialise( true, "openDarkEngine" );
		
		// Initialise resources
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
		
		mDarkSMFactory = new DarkSceneManagerFactory();

		// Register
		Root::getSingleton().addSceneManagerFactory(mDarkSMFactory);
			
		mRoot->createSceneManager(ST_INTERIOR, "DarkSceneManager");
		
		return true;
	}
	
	void GameStateManager::setupInputSystem() {
		// Copied from the OIS Example. Thanks
		// Setup basic variables
		OIS::ParamList paramList;    
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
	
		// Get window handle
		mRenderWindow->getCustomAttribute( "WINDOW", &windowHnd );
			
		// Fill parameter list
		windowHndStr << (unsigned int) windowHnd;
		paramList.insert( std::make_pair( std::string( "WINDOW" ), windowHndStr.str() ) );
	
		/* // Non-exclusive input - for debugging purposes
		#if defined OIS_WIN32_PLATFORM
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
		paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
		paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
		#elif defined OIS_LINUX_PLATFORM
		paramList.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
		paramList.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
		paramList.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
		paramList.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
		#endif
		*/
		
		// Create inputsystem
		mInputSystem = OIS::InputManager::createInputSystem( paramList );
	
		// If possible create a buffered keyboard
		if( mInputSystem->numKeyBoards() > 0 ) {
			mKeyboard = static_cast<OIS::Keyboard*>( mInputSystem->createInputObject( OIS::OISKeyboard, true ) );
			mKeyboard->setEventCallback( this );
		}
	
		// If possible create a buffered mouse
		if( mInputSystem->numMice() > 0 ) {
			mMouse = static_cast<OIS::Mouse*>( mInputSystem->createInputObject( OIS::OISMouse, true ) );
			mMouse->setEventCallback( this );
	
			// Get window size
			unsigned int width, height, depth;
			int left, top;
			mRenderWindow->getMetrics( width, height, depth, left, top );
	
			// Set mouse region
			const OIS::MouseState &mouseState = mMouse->getMouseState();
    			mouseState.width  = width;
    			mouseState.height = height;
		}
	}
	
	void GameStateManager::captureInputs() {
		if( mMouse ) {
		        mMouse->capture();
    		}

		if( mKeyboard ) {
        		mKeyboard->capture();
    		}
	}
	
	bool GameStateManager::keyPressed( const OIS::KeyEvent &e ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->keyPressed(e);
		}
	}
	
	bool GameStateManager::keyReleased( const OIS::KeyEvent &e ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->keyReleased(e);
		}
	}

	bool GameStateManager::mouseMoved( const OIS::MouseEvent &e ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->mouseMoved(e);
		}
	}
	
	bool GameStateManager::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->mousePressed(e, id);
		}
	}
	
	bool GameStateManager::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->mouseReleased(e, id);
		}
	}

}
