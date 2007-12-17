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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __GAMESTATEMANAGER_H
#define __GAMESTATEMANAGER_H

#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISInputManager.h>

#include <OgreRenderWindow.h>

#include <stack>

#include "OpdeSingleton.h"
#include "GameState.h"
#include "stdlog.h"
#include "filelog.h"
#include "ConsoleBackend.h"
#include "OpdeServiceManager.h"
#include "ConfigService.h"
#include "InputService.h"
#include "DVariant.h"

#include "DTypeScriptLoader.h"
#include "PLDefScriptLoader.h"

namespace Opde {

	/** The game state manager. A main piece of code, which controls the flow of the program. Also dispatcher for input device events.
	* A state is a class inheriting GameState abstract class, which controls a single state of the game. GameState's instance
	* then receives events from keyboard and other input devices, event for frame update, etc.
	*
	* @note This class should implements a custom game loop.
	*
	* @see GameState
	* @todo add OIS::JoyStickListener
	*/
	class GameStateManager : public Singleton<GameStateManager>, public DirectInputListener {
        public:
            GameStateManager();
            ~GameStateManager();

            // Singleton related
            static GameStateManager& getSingleton(void);
            static GameStateManager* getSingletonPtr(void);

			/// Terminates the execution of the game loop
			void terminate();

			/** Pushes a new state to the stack, and calls start() on this new state
			* If the stack was not empty before, suspend() is called on previous top
			*/
			void pushState(GameState* state);

			/** Pops the topmost state from stack, if possible. Calls exit() on such state
			*
			*/
			void popState();

			/** Initialize the state manager, then run the loop with the given state. Initializes ogre, resources, input system, etc.
			* @return true if game should procede, false otherwise */
			bool run();
		protected:
			/** Registers all the service factories */
			void registerServiceFactories();

			/** Loads the resources from the resources.cfg */
			void setupResources(void);

			/** Setup the OIS input system. */
			void setupInputSystem();

			bool keyPressed( const OIS::KeyEvent &e );
			bool keyReleased( const OIS::KeyEvent &e );

			bool mouseMoved( const OIS::MouseEvent &e );
			bool mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id );
			bool mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id );


			typedef std::stack<GameState*> StateStack;

			/// Stack of the game states
			StateStack mStateStack;

			/// the game loop should end if true
			bool	mTerminate;

			/// last frame's time
			unsigned long mTimeLastFrame;

			///  Stderr logger
			StdLog* mStdLog;
			FileLog* mFileLog;

			ConsoleBackend* mConsoleBackend;

			Logger *mLogger;

			// --- Ogre instances
			Ogre::Root *mRoot;

			// Service manager handle
			ServiceManager* mServiceMgr;

			// Loader for the DType scripts
			DTypeScriptLoader* mDTypeScriptLdr;

			// Loader for the PLDef scripts
			PLDefScriptLoader* mPLDefScriptLdr;

			// config service
			ConfigServicePtr mConfigService;

			InputServicePtr mInputService;
	};

}

#endif
