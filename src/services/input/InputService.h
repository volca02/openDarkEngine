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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __INPUTSERVICE_H
#define __INPUTSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "DVariant.h"
#include "InputEventMapper.h"
#include "Callback.h"
#include "ConfigService.h"
#include "RenderService.h"
#include "LoopService.h"

#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OISInputManager.h>

#include <deque>

namespace Opde {

	// Damn this is just a template ok? I don't mean to use this, but there has to be a way to specify the listener type/mask interest (keypress, every key scan, key release, etc)
	// First, I have to specify if the input servis has to be stateful or stateless
	typedef enum {
			IET_KEYBOARD_PRESS,
			IET_KEYBOARD_HOLD,
			IET_KEYBOARD_RELEASE,
			IET_MOUSE_MOVE,
			IET_MOUSE_PRESS,
			IET_MOUSE_HOLD,
			IET_MOUSE_RELEASE,
			IET_JOYSTICK_MOVE,
			IET_JOYSTICK_PRESS,
			IET_JOYSTICK_HOLD,
			IET_JOYSTICK_RELEASE
	} InputEventType;

	typedef struct {
		InputEventType event;
		 // unmapped command, or empty
		std::string command;
		// Parameters of the command
		DVariant params;
	} InputEventMsg;

	/// The state of input modifiers
	typedef enum {
		/// Shift key modifier
		IST_SHIFT = 1,
		/// Alt key modifier
		IST_ALT  = 2,
		/// Control key modifier
		IST_CTRL = 4
	} InputModifierState;

	/// The input mode - direct or translated to the commands
	typedef enum {
		/// Direct mode
		IM_DIRECT = 1,
		/// Translated mode
		IM_MAPPED
	} InputMode;

	// Imports of OIS definitions to Opde namespace
	/*typedef OIS::KeyEvent KeyEvent;
	typedef OIS::MouseEvent MouseEvent;
	typedef OIS::MouseButtonID MouseButtonID;*/

	/// Listener for the direct - unfiltered events. Typically one per application (GUIService for example)
	class DirectInputListener {
		public:
			virtual bool keyPressed( const OIS::KeyEvent &e ) = 0;
			virtual bool keyReleased( const OIS::KeyEvent &e ) = 0;

    		virtual bool mouseMoved( const OIS::MouseEvent &e ) = 0;
    		virtual bool mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) = 0;
    		virtual bool mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) = 0;
	};


	/** @brief Input service - service which handles user input, and user input mapping */
	class InputService : public Service, public OIS::KeyListener, public OIS::MouseListener, public LoopClient {
		public:
			InputService(ServiceManager *manager, const std::string& name);
			virtual ~InputService();

			/// Creates bind context (e.g. a switchable context, that maps events to commands if IM_MAPPED is active, using the mapper of this context)
			void createBindContext(const std::string& ctx);

			/// returns true if the key text is a valid key name
			bool isKeyTextValid(std::string& txt);

			/// returns ois::KeyCode code for the key text, or KC_UNASSIGNED if not found
			OIS::KeyCode getKeyTextCode(std::string& txt);

			/// returns the mapped key text for the OIS key code
			std::string getKeyText(OIS::KeyCode kc);

			/// Input Service command. If you call this one a line-by-line on .BND file, it should catch up and be ready to operate with the settings
			DVariant command(const std::string& command);

			/// Set a current bind context
			void setBindContext(const std::string& context);

			/// Sets the input mode to either direct or mapped (IM_DIRECT/IM_MAPPED)
			void setInputMode(InputMode mode) { mInputMode = mode; };

			/** Loads a bindings file, possibly rebinding the current bindings
			* @param dcontext The default context for bind commands not preceeded with the context information. Defaults to the current context if not specified
			*/
			void loadBNDFile(const std::string& filename);

			/// TODO:	void saveBNDFile(const std::string& filename);

			/// Variable getter (for mouse senitivity, etc)
			DVariant getVariable(const std::string& var);

			/// Variable setter (for mouse senitivity, etc)
			void setVariable(const std::string& var, const DVariant& val);

			/** Validates the event string (checks for format "event[+modifier]")
			* @return true if the string is valid, true otherwise
			*/
			bool validateEventString(const std::string& ev);

			/// Definition of the command listener
			typedef Callback< InputEventMsg > Listener;

			/// Definition of the command listener Shared Pointer
			typedef shared_ptr< Listener > ListenerPtr;

			/// Registers a command listener
			void registerCommandTrap(const std::string& command, ListenerPtr listener);

			/// Unregisters a command listener
			void unregisterCommandTrap(ListenerPtr listener);
			
			/// Unregisters a command listener by it's name
			void unregisterCommandTrap(const std::string& command);

			/// Sets a direct listener (only notified when IM_DIRECT is active)
			void setDirectListener(DirectInputListener* listener) { mDirectListener = listener; };

			/// Clears the direct listener mapping
			void unsetDirectListener() { mDirectListener = NULL; };

			/** Capture the inputs. Should be called every frame (Temporary code, will be removed after loop service is done) */
			void captureInputs();

		protected:
            bool init();
            void bootstrapFinished();
            void shutdown();

			void loopStep(float deltaTime);

			/// registers OIS::KeyCode to textual representation and inverse mappings
			void registerValidKey(OIS::KeyCode kc, const std::string& txt);

			/// Attaches the ctrl, alt and shift texts if they are pressed
			void attachModifiers(std::string& tgt);

			/// Calls a trap for a command
			/// @returns true if the command was found, false otherwise
			bool callCommandTrap(const InputEventMsg& msg);

			/// returns true if the command trap exists for a given command
			bool hasCommandTrap(const std::string& cmd);

			/// Splits the given command on first whitespace to command an parameters parts
			std::pair<std::string, std::string> splitCommand(const std::string& cmd) const;

			/// Processes the received key event with current mapper, and if it finds a match, sends an event
			void processKeyEvent(const OIS::KeyEvent &e, InputEventType t);


			// ---- OIS input events ----
			bool keyPressed( const OIS::KeyEvent &e );
			bool keyReleased( const OIS::KeyEvent &e );

    		bool mouseMoved( const OIS::MouseEvent &e );
    		bool mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id );
    		bool mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id );

			/// Finds a mapper (Or NULL) for the specified context
            InputEventMapperPtr findMapperForContext(const std::string& ctx);

			/// Strips a comment (any text after ';' including that character)
			std::string stripComment(const std::string& cmd);

			/// Named context to an event mapper map
			typedef std::map< std::string, InputEventMapperPtr > ContextToMapper;

			/// string variable name to variant map
			typedef std::map< std::string, DVariant > ValueMap;

			/// map of OIS::KeyCode to the code text
			typedef std::map< OIS::KeyCode, std::string > KeyMap; // (lower case please)
			
			/// map of the command text to the ois key code
			typedef std::map< std::string, OIS::KeyCode > ReverseKeyMap; // (lower case please)

			/// map of command text to the handling listener
			typedef std::map< std::string, ListenerPtr > ListenerMap;
			
			/// Set of commands that receive event on hold, every refresh
			typedef std::set< std::string > CommandSet;
			
			/// map of the context name to the mapper
			ContextToMapper mMappers;
			
			/// Variable list
			ValueMap mVariables;
			
			/// Key map
			KeyMap mKeyMap;
			
			/// Reverse key map
			ReverseKeyMap mReverseKeyMap;

			/// Current mapper
			InputEventMapperPtr mCurrentMapper;
			
			/// Current input mode
			InputMode mInputMode;

			/// Map of the command trappers
			ListenerMap mCommandTraps;

			/// Set of commands that receive events every refresh if the button is holded
			CommandSet mOnPressCommands;

			/// Current direct listener 
			/// TODO: Maybe this will become a stack
			DirectInputListener* mDirectListener;

			// Input system related objects
			/// OIS input system
			OIS::InputManager *mInputSystem;
			/// OIS mouse pointer
			OIS::Mouse        *mMouse;
			/// OIS keyboard pointer
    		OIS::Keyboard     *mKeyboard;

			/// Renderer window we listen on
			Ogre::RenderWindow* mRenderWindow;

			/// Config service pointer
			ConfigServicePtr mConfigService;
			
			/// Render service pointer
			RenderServicePtr mRenderService;
			
			/// Loop service pointer
			LoopServicePtr mLoopService;
	};

	/// Shared pointer to input service
	typedef shared_ptr<InputService> InputServicePtr;

	/// Factory for the InputService objects
	class InputServiceFactory : public ServiceFactory {
		public:
			InputServiceFactory();
			~InputServiceFactory() {};

			/** Creates a InputService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask(); 
		private:
			static std::string mName;
	};
}


#endif
