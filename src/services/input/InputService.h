/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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

#include "config.h"
#include "ServiceCommon.h"

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
#include <OISJoyStick.h>
#include <OISKeyboard.h>
#include <OISInputManager.h>

#include <deque>

namespace Opde {

	/// Input event types
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
			IET_JOYSTICK_RELEASE,
			/// Plain command call not caused by user input directly
			IET_COMMAND_CALL
	} InputEventType;

#define SHIFT_MOD			(1 << 31)
#define CTRL_MOD			(1 << 30)
#define ALT_MOD				(1 << 29)
#define DARK_JOY_EVENT		(1 << 28)
#define DARK_MOUSE_EVENT	(1 << 27)

	enum DarkJoyStickEvents
	{
		JOY_AXISR = DARK_JOY_EVENT,
		JOY_AXISX,
		JOY_AXISY,
		JOY_HAT_UP,
		JOY_HAT_DOWN,
		JOY_HAT_RIGHT,
		JOY_HAT_LEFT,
		JOY_1,
		JOY_2,
		JOY_3,
		JOY_4,
		JOY_5,
		JOY_6,
		JOY_7,
		JOY_8,
		JOY_9,
		JOY_10
	};

	enum DarkMouseEvents
	{
		MOUSE1 = DARK_MOUSE_EVENT + OIS::MB_Left,
		MOUSE2 = DARK_MOUSE_EVENT + OIS::MB_Right,
		MOUSE3 = DARK_MOUSE_EVENT + OIS::MB_Middle,
		MOUSE4 = DARK_MOUSE_EVENT + OIS::MB_Button3,
		MOUSE5 = DARK_MOUSE_EVENT + OIS::MB_Button4,
		MOUSE6 = DARK_MOUSE_EVENT + OIS::MB_Button5,
		MOUSE7 = DARK_MOUSE_EVENT + OIS::MB_Button6,
		MOUSE8 = DARK_MOUSE_EVENT + OIS::MB_Button7,
		MOUSE_AXISX,
		MOUSE_AXISY,
		MOUSE_WHEEL
	};

	/// Input event
	typedef struct
	{
		InputEventType event;
		 // unmapped command, or empty
		std::string command;
		// Parameters of the command
		DVariant params;
	} InputEventMsg;

	/// The state of input modifiers
	typedef enum
	{
		/// Shift key modifier
		IST_SHIFT = 1,
		/// Alt key modifier
		IST_ALT  = 2,
		/// Control key modifier
		IST_CTRL = 4
	} InputModifierState;

	/// The input mode - direct or translated to the commands
	typedef enum
	{
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
	class DirectInputListener
	{
		public:
			virtual ~DirectInputListener() {};

			virtual bool keyPressed(const OIS::KeyEvent &e) = 0;
			virtual bool keyReleased(const OIS::KeyEvent &e) = 0;

			virtual bool mouseMoved(const OIS::MouseEvent &e) = 0;
			virtual bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;
			virtual bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;

			virtual bool povMoved(const OIS::JoyStickEvent &e, int pov) = 0;
			virtual bool axisMoved(const OIS::JoyStickEvent &arg, int axis) = 0;
			virtual bool buttonPressed(const OIS::JoyStickEvent &arg, int button) = 0;
			virtual bool buttonReleased(const OIS::JoyStickEvent &arg, int button) = 0;
	};


	/** @brief Input service - service which handles user input, and user input mapping
	 * @todo Tab completion for the service (preferably including variables)
	 * @todo Variable registration, descriptions, enumeration (So one can get list of variables, get their value and description)
	 * @todo Mouse axis handling
	 * @todo Joystick handling
	 * @todo +/without + binding modes - how they differ, how to implement the one with key repeat...
	 * @todo BND file writing ability (needs cooperation with nonexistent platform service)
	 */
	class OPDELIB_EXPORT InputService : public ServiceImpl<InputService>, public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener, public LoopClient
	{
		public:
			InputService(ServiceManager *manager, const std::string& name);
			virtual ~InputService();

			/// Creates bind context (e.g. a switchable context, that maps events to commands if IM_MAPPED is active, using the mapper of this context)
			void createBindContext(const std::string& ctx);

			/// Set a current bind context
			void setBindContext(const std::string& context);

			/// Replaces all occurences of $variable with it's value
			std::string fillVariables(const std::string& src) const;

			/// Sets the input mode to either direct or mapped (IM_DIRECT/IM_MAPPED)
			void setInputMode(InputMode mode) { mInputMode = mode; };

			/** Loads a bindings file, possibly rebinding the current bindings
			 * @param filename The filename of the bnd file to load
			* @todo dcontext The default context for bind commands not preceeded with the context information. Defaults to the current context if not specified
			*/
			bool loadBNDFile(const std::string& filename);

			/// Adds a command to the pool
			DVariant processCommand(const std::string& CommandString);

			/// TODO:	void saveBNDFile(const std::string& filename);

			/// Variable getter (for mouse senitivity, etc)
			DVariant getVariable(const std::string& var);

			/// Variable setter (for mouse senitivity, etc)
			void setVariable(const std::string& var, const DVariant& val);

			/// Definition of the command listener
			typedef Callback< InputEventMsg > Listener;

			/// Definition of the command listener Shared Pointer
			typedef shared_ptr< Listener > ListenerPtr;

			/// Registers a command listener
			void registerCommandTrap(const std::string& command, const ListenerPtr& listener);

			/// Unregisters a command listener
			void unregisterCommandTrap(const ListenerPtr& listener);

			/// Unregisters a command listener by it's name
			void unregisterCommandTrap(const std::string& command);

			/** registers a command alias
			* @param alias The alias to use
			* @param command the command that gets executed when the alias is encountered
			*/
			void registerCommandAlias(const std::string& alias, const std::string& command);

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

			/// Processes key repeats, calls processKeyEvent on appropriate delays
			void processKeyRepeat(float deltaTime);

			/// Initilizes the key name to code map
			void initKeyMap();

			/// Unmaps the string into ois keycode
			unsigned int mapToOISCode(const std::string &key) const;

			/// registers (int)OIS::KeyCode to textual representation and inverse mappings
			void registerValidKey(int kc, const std::string& txt);

			/** Calls a trap for a command
			* @warning Modifies the msg parameter upon dealiasing
			* @returns true if the command was found, false otherwise
			*/
			bool callCommandTrap(InputEventMsg& msg);

			/// returns true if the command trap exists for a given command
			bool hasCommandTrap(const std::string& cmd);

			/// Splits the given command on first whitespace to command an parameters parts
			std::pair<std::string, std::string> splitCommand(const std::string& cmd) const;

			/// Logs all available commands
			void logCommands();

			/// Logs all available variables and their values
			void logVariables();

			/// Processes the received key event with current mapper, and if it finds a match, sends an event
			void processKeyEvent(unsigned int keyCode, InputEventType t);

			/// Processes joystick or mouse event (axis movement, etc)
			void processJoyMouseEvent(unsigned int Id, InputEventType Event);

			// ---- OIS input events ----
			bool keyPressed(const OIS::KeyEvent &e);
			bool keyReleased(const OIS::KeyEvent &e);

			bool mouseMoved(const OIS::MouseEvent &e);
			bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
			bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);

			bool axisMoved(const OIS::JoyStickEvent &,int);
			bool buttonPressed(const OIS::JoyStickEvent &,int);
			bool buttonReleased(const OIS::JoyStickEvent &,int);
			bool povMoved(const OIS::JoyStickEvent &, int);

			/// Finds a mapper (Or NULL) for the specified context
			InputEventMapper* findMapperForContext(const std::string& ctx);

			/// Strips a comment (any text after ';' including that character)
			std::string stripComment(const std::string& cmd);

			/// Defines a new input binding in specified mapper
			void addBinding(const std::string& keys, const std::string& command, InputEventMapper* mapper);

			/** Gives a command found for the given alias, if any
			* @param alias The aliased command
			* @param command the string that gets filled with the dealiased version of the command
			* @return true if the alias was found and command was dealiased
			*/
			bool dealiasCommand(const std::string& alias, std::string& command);


			/// Named context to an event mapper map
			typedef std::map< std::string, InputEventMapper* > ContextToMapper;

			/// string variable name to variant map
			typedef std::map< std::string, DVariant > ValueMap;

			/// map of (int)OIS::KeyCode to the code text
			typedef std::map<unsigned int, std::string> KeyMap; // (lower case please)

			/// map of the command text to the (int) ois key code
			typedef std::map<std::string, int> ReverseKeyMap; // (lower case please)

			/// map of command text to the handling listener
			typedef std::map< std::string, ListenerPtr > ListenerMap;

			/// string variable name to dealiased value
			typedef std::map< std::string, std::string > AliasMap;

			/// map of the context name to the mapper
			ContextToMapper mMappers;

			/// Variable list
			ValueMap mVariables;

			/// Key map
			KeyMap mKeyMap;

			/// Reverse key map
			ReverseKeyMap mReverseKeyMap;

			/// Current mapper
			InputEventMapper* mCurrentMapper;
			InputEventMapper* mDefaultMapper;

			/// Current input mode
			InputMode mInputMode;

			/// Map of the command trappers
			ListenerMap mCommandTraps;

			/// Current direct listener
			DirectInputListener* mDirectListener;

			/// Key Repeat: Initial delay (between first and second key repeats)
			float mInitialDelay;
			/// Key Repeat: Repeat delay (between second and latter key repeats)
			float mRepeatDelay;
			/// Key Repeat: Current key time
			float mKeyPressTime;
			/// Key Repeat: Current target delay time
			float mCurrentDelay;
			/// Key Repeat: Current key pressed
			OIS::KeyCode mCurrentKey;

			// Input system related objects
			/// OIS input system
			OIS::InputManager *mInputSystem;
			/// OIS mouse pointer
			OIS::Mouse        *mMouse;
			/// OIS keyboard pointer
			OIS::Keyboard     *mKeyboard;
			/// OIS joystick pointer
			OIS::JoyStick     *mJoystick;

			/// Renderer window we listen on
			Ogre::RenderWindow* mRenderWindow;

			/// Config service pointer
			ConfigServicePtr mConfigService;

			/// Render service pointer
			RenderServicePtr mRenderService;

			/// Loop service pointer
			LoopServicePtr mLoopService;

			/// Non-exclusive input
			bool mNonExclusive;

			/// Aliases to commands
			AliasMap mCommandAliases;
	};

	/// Shared pointer to input service
	typedef shared_ptr<InputService> InputServicePtr;

	/// Factory for the InputService objects
	class OPDELIB_EXPORT InputServiceFactory : public ServiceFactory
	{
		public:
			InputServiceFactory();
			~InputServiceFactory() {};

			/** Creates a InputService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();

			virtual const size_t getSID();
		private:
			static std::string mName;
	};
}


#endif
