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


#ifndef __GAMESTATE_H
#define __GAMESTATE_H

#include "RefCounted.h"
#include <OIS.h>

namespace Opde {
	
	/** A game state. Abstract class which is filled with code doing stuff while state is on top of the queue.
	* It is guranteed, that the state will receive start() on becoming a new top state, suspend() when a new state covers this as new active, resume() when state
	* becomes active again, and exit() when the state is being removed from the queue.
	* @note To give the control to another state, when exiting this one, just pushState the new state in the exit() method
	*/
	class GameState : public RefCounted {
		public:
			/** Constructor */
			GameState();
			
			//---------- Events from StateManager for particular events
			/** Do a state startup. Called if the state becomes the new active game state (added to the state stack) */
			virtual void start() = 0;
			
			/** Do a state cleanup. Called when the state is removed from the stack */
			virtual void exit() = 0; 
			
			/** Do suspend the game state. Called when another state takes the activity */
			virtual void suspend() = 0;
			
			/** Do a state resume. Called when the state becomes active again, being not covered by another state */
			virtual void resume() = 0; 
			
			/** Update the state, given timePassed time since last update call */
			virtual void update(unsigned long timePassed) = 0;
			
			//---------- input events releted methods
			
			/** Keyboard key was pressed event. */
			virtual bool keyPressed( const OIS::KeyEvent &e ) = 0;
			
			/** Keyboard key was released event. */
			virtual bool keyReleased( const OIS::KeyEvent &e ) = 0;

			/** Mouse moved event. */
			virtual bool mouseMoved( const OIS::MouseEvent &e ) = 0;
			
			/** Mouse button was pressed event. */
			virtual bool mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) = 0;
			
			/** Mouse button was released event. */
			virtual bool mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) = 0;
			
			//---------- StateManager releted methods
			
			/** Request the StateManager to push in a new state */
			void pushState(GameState* state);
			
			/** Request the StateManager to pop a state */
			void popState();
			
			/** Request the StateManager to terminate */ 
			void requestTermination();
	};	
	
}

#endif
