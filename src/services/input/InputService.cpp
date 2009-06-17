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

#include "ServiceCommon.h"
#include "InputService.h"
#include "OpdeException.h"
#include "logger.h"
#include "StringTokenizer.h"

#include <OgreResourceGroupManager.h>
#include <OgreRenderWindow.h>

using namespace std;
using namespace Ogre;
using namespace OIS;

namespace Opde {

    /*-----------------------------------------------------*/
    /*-------------------- InputService -------------------*/
    /*-----------------------------------------------------*/
    InputService::InputService(ServiceManager *manager, const std::string& name) :
			Service(manager, name),
			mInputMode(IM_MAPPED),
			mDirectListener(NULL),
			mMouse(NULL),
			mKeyboard(NULL),
			mJoystick(NULL) 
	{

    	// Loop client definition
    	mLoopClientDef.id = LOOPCLIENT_ID_INPUT;
    	mLoopClientDef.mask = LOOPMODE_INPUT;
    	mLoopClientDef.priority = LOOPCLIENT_PRIORITY_INPUT;
    	mLoopClientDef.name = mName;

		mCurrentMapper = InputEventMapperPtr(new InputEventMapper(this));

		mCurrentKey = OIS::KC_UNASSIGNED;
		mKeyPressTime = 0.0f;
		mInitialDelay = 0.4f; // TODO: Read these from the config service
		mRepeatDelay = 0.3f;
		
		mNonExclusive = false;
    }

    //------------------------------------------------------
    InputService::~InputService() 
	{
    	mMappers.clear();
    	mCurrentMapper.setNull();

		if (mInputSystem) 
		{
			if( mMouse ) 
			{
				mInputSystem->destroyInputObject( mMouse );
				mMouse = NULL;
			}

			if( mKeyboard ) 
			{
				mInputSystem->destroyInputObject( mKeyboard );
				mKeyboard = NULL;
			}

			if( mJoystick ) 
			{
				mInputSystem->destroyInputObject( mJoystick );
				mJoystick = NULL;
			}

			mInputSystem->destroyInputSystem(mInputSystem);
			mInputSystem = NULL;
		}


		if (!mLoopService.isNull())
			mLoopService->removeLoopClient(this);

		mRenderService.setNull();
    }

	//------------------------------------------------------
	void InputService::registerValidKey(int kc, const std::string& txt) 
	{
		mKeyMap.insert(make_pair(kc, txt));
		mReverseKeyMap.insert(make_pair(txt, kc));
	}

	//------------------------------------------------------
	void InputService::initKeyMap()
	{
		registerValidKey(KC_ESCAPE, "esc");

		registerValidKey(KC_1, "1");
		registerValidKey(KC_1 | SHIFT_MOD, "!");
		registerValidKey(KC_2, "2");
		registerValidKey(KC_2 | SHIFT_MOD, "@");
		registerValidKey(KC_3, "3");
		registerValidKey(KC_3 | SHIFT_MOD, "#");
		registerValidKey(KC_4, "4");
		registerValidKey(KC_4 | SHIFT_MOD, "$");
		registerValidKey(KC_5, "5");
		registerValidKey(KC_5 | SHIFT_MOD, "%");
		registerValidKey(KC_6, "6");
		registerValidKey(KC_6 | SHIFT_MOD, "^");
		registerValidKey(KC_7, "7");
		registerValidKey(KC_7 | SHIFT_MOD, "&");
		registerValidKey(KC_8, "8");
		registerValidKey(KC_8 | SHIFT_MOD, "*");
		registerValidKey(KC_9, "9");
		registerValidKey(KC_9 | SHIFT_MOD, "(");
		registerValidKey(KC_0, "0");
		registerValidKey(KC_0 | SHIFT_MOD, ")");

		registerValidKey(KC_MINUS, "-");
		registerValidKey(KC_MINUS | SHIFT_MOD, "_");
		registerValidKey(KC_EQUALS, "=");
		registerValidKey(KC_EQUALS | SHIFT_MOD, "+");
		registerValidKey(KC_BACK, "backspace");
		registerValidKey(KC_TAB, "tab");

		registerValidKey(KC_Q, "q");
		registerValidKey(KC_W, "w");
		registerValidKey(KC_E, "e");
		registerValidKey(KC_R, "r");
		registerValidKey(KC_T, "t");
		registerValidKey(KC_Y, "y");
		registerValidKey(KC_U, "u");
		registerValidKey(KC_I, "i");
		registerValidKey(KC_O, "o");
		registerValidKey(KC_P, "p");

		registerValidKey(KC_LBRACKET, "[");
		registerValidKey(KC_LBRACKET | SHIFT_MOD, "{");
		registerValidKey(KC_RBRACKET, "]");
		registerValidKey(KC_RBRACKET | SHIFT_MOD, "}");
		registerValidKey(KC_RETURN, "enter");
		registerValidKey(KC_LCONTROL, "ctrl");

		registerValidKey(KC_A, "a");
		registerValidKey(KC_S, "s");
		registerValidKey(KC_D, "d");
		registerValidKey(KC_F, "f");
		registerValidKey(KC_G, "g");
		registerValidKey(KC_H, "h");
		registerValidKey(KC_J, "j");
		registerValidKey(KC_K, "k");
		registerValidKey(KC_L, "l");
		registerValidKey(KC_SEMICOLON, ";");
		registerValidKey(KC_SEMICOLON | SHIFT_MOD, ":");
		registerValidKey(KC_APOSTROPHE, "'");
		registerValidKey(KC_APOSTROPHE | SHIFT_MOD, "\"");
		registerValidKey(KC_LSHIFT, "shift");
		registerValidKey(KC_BACKSLASH, "\\");
		registerValidKey(KC_BACKSLASH | SHIFT_MOD, "|");
		registerValidKey(KC_Z, "z");
		registerValidKey(KC_X, "x");
		registerValidKey(KC_C, "c");
		registerValidKey(KC_V, "v");
		registerValidKey(KC_B, "b");
		registerValidKey(KC_N, "n");
		registerValidKey(KC_M, "m");
		registerValidKey(KC_COMMA, ",");
		registerValidKey(KC_COMMA | SHIFT_MOD, "<");
		registerValidKey(KC_PERIOD, ".");
		registerValidKey(KC_PERIOD | SHIFT_MOD, ">");
		registerValidKey(KC_SLASH, "/");
		registerValidKey(KC_SLASH, "keypad_slash");
		registerValidKey(KC_SLASH | SHIFT_MOD, "?");
		registerValidKey(KC_RSHIFT, "shift");
		registerValidKey(KC_MULTIPLY, "keypad_star");
		registerValidKey(KC_LMENU, "alt");
		registerValidKey(KC_SPACE, "space");
		registerValidKey(KC_F1, "f1");
		registerValidKey(KC_F2, "f2");
		registerValidKey(KC_F3, "f3");
		registerValidKey(KC_F4, "f4");
		registerValidKey(KC_F5, "f5");
		registerValidKey(KC_F6, "f6");
		registerValidKey(KC_F7, "f7");
		registerValidKey(KC_F8, "f8");
		registerValidKey(KC_F9, "f9");
		registerValidKey(KC_F10, "f10");
		registerValidKey(KC_NUMLOCK, "numlock");
		registerValidKey(KC_SCROLL, "scroll");
		registerValidKey(KC_NUMPAD7, "keypad_home");
		registerValidKey(KC_NUMPAD8, "keypad_up");
		registerValidKey(KC_NUMPAD9, "keypad_pgup");
		registerValidKey(KC_SUBTRACT, "keypad_minus");
		registerValidKey(KC_NUMPAD4, "keypad_left");
		registerValidKey(KC_NUMPAD5, "keypad_center");
		registerValidKey(KC_NUMPAD6, "keypad_right");
		registerValidKey(KC_ADD, "keypad_plus");
		registerValidKey(KC_NUMPAD1, "keypad_end");
		registerValidKey(KC_NUMPAD2, "keypad_down");
		registerValidKey(KC_NUMPAD3, "keypad_pgdn");
		registerValidKey(KC_NUMPAD0, "keypad_ins");
		registerValidKey(KC_DECIMAL, "keypad_del");
		registerValidKey(KC_F11, "f11");
		registerValidKey(KC_F12, "f12");
		registerValidKey(KC_F13, "f13");
		registerValidKey(KC_F14, "f14");
		registerValidKey(KC_F15, "f15");

		registerValidKey(KC_NUMPADENTER, "keypad_enter");
		registerValidKey(KC_RMENU, "alt");
		registerValidKey(KC_HOME, "home");
		registerValidKey(KC_UP, "up");
		registerValidKey(KC_PGUP, "pgup");
		registerValidKey(KC_LEFT, "left");
		registerValidKey(KC_RIGHT, "right");
		registerValidKey(KC_END, "end");
		registerValidKey(KC_DOWN, "down");
		registerValidKey(KC_PGDOWN, "pgdn");
		registerValidKey(KC_INSERT, "ins");
		registerValidKey(KC_DELETE, "del");
		registerValidKey(KC_GRAVE, "`");
		registerValidKey(KC_GRAVE | SHIFT_MOD, "~");
		registerValidKey(KC_SYSRQ, "print_screen");
		registerValidKey(joy_axisr, "joy_axisr");
		registerValidKey(joy_axisx, "joy_axisx");
		registerValidKey(joy_axisy, "joy_axisy");
		registerValidKey(joy_hat_up, "joy_hatup");
		registerValidKey(joy_hat_down, "joy_hatdn");
		registerValidKey(joy_hat_right, "joy_hatrt");
		registerValidKey(joy_hat_left, "joy_hatlt");
		registerValidKey(joy_1, "joy1");
		registerValidKey(joy_2, "joy2");
		registerValidKey(joy_3, "joy3");
		registerValidKey(joy_4, "joy4");
		registerValidKey(joy_5, "joy5");
		registerValidKey(joy_6, "joy6");
		registerValidKey(joy_7, "joy7");
		registerValidKey(joy_8, "joy8");
		registerValidKey(joy_9, "joy9");
		registerValidKey(joy_10, "joy10");
		registerValidKey(Mouse1, "mouse1");
		registerValidKey(Mouse2, "mouse2");
		registerValidKey(Mouse3, "mouse3");
		registerValidKey(Mouse4, "mouse4");
		registerValidKey(Mouse5, "mouse5");
		registerValidKey(Mouse6, "mouse6");
		registerValidKey(Mouse7, "mouse7");
		registerValidKey(Mouse8, "mouse8");
		registerValidKey(Mouse_axisx, "mouse_axisx");
		registerValidKey(Mouse_axisy, "mouse_axisy");
		registerValidKey(Mouse_wheel, "mouse_wheel");
	}

	//------------------------------------------------------
	void InputService::tokenize(std::string inString, std::vector<std::string> &outVector, char token)
	{
		size_t OldOffset = 0, NewOffset;

		while(OldOffset < inString.length()) {
			NewOffset = inString.find(token, OldOffset);
			if( (NewOffset == string::npos) || (inString.at(OldOffset) == 0x22))
				NewOffset = inString.length();
			outVector.push_back (inString.substr(OldOffset, NewOffset - OldOffset));
			OldOffset = NewOffset + 1;
		};
	}

	//------------------------------------------------------
	unsigned int InputService::mapToOISCode(std::string key) const
	{
		unsigned int Code;
		ReverseKeyMap::const_iterator it = mReverseKeyMap.find(key);

		if (it != mReverseKeyMap.end())
			Code = it->second;
		else
			Code = KC_UNASSIGNED;	//If we get here, then there is a key name that DarkEngine creates that we didn't cover

		return Code;
	}

	//------------------------------------------------------
	void InputService::addBindCommand(const ContentsVector& command)
	{
		ContentsVector contents;
		unsigned int modifier = 0;
		std::string key, keys = command.at(1);

		if((keys.length() > 1) && (keys.find('+') != string::npos))
		{
			contents.clear();
			tokenize(keys, contents, '+');
			for(ContentsVector::const_iterator content = contents.begin(); content != contents.end(); content++)
			{
				std::string ContentStr = *content;
				if (ContentStr == "shift")
					modifier |= SHIFT_MOD;
				else if (ContentStr == "ctrl")
					modifier |= CTRL_MOD;
				else if (ContentStr == "alt")
					modifier |= ALT_MOD;
				else
					key = ContentStr;
			}
		}
		else
			key = keys;
		
		const unsigned int code = mapToOISCode(key);
		mCommandMap.insert(make_pair(code | modifier, stripComment(command.at(2))));
	}

	//------------------------------------------------------
	DVariant InputService::processCommand(const std::string& commandStr)
	{
		ContentsVector contents, command;
		
		std::string cstr;
		cstr = stripComment(commandStr);
		
		if((commandStr.length() == 0) || (commandStr.at(0) == ';'))
			return false;
		
		std::transform(cstr.begin(), cstr.end(), cstr.begin(), ::tolower);	//Lowercase
		
		WhitespaceStringTokenizer tok(cstr, false);
	
		while (!tok.end()) 
			command.push_back(tok.next());
		
		if(command.at(0) == "bind")
		{
			addBindCommand(command);
			return true;
		}
		else if(command.at(0) == "echo")
			return commandStr.substr(5);
		else if(command.at(0) == "set")
		{
			setVariable(command.at(1), command.at(2));
			return true;
		}
		
		setVariable(command.at(0), command.at(1));
		return true;
	}

	//------------------------------------------------------
	bool InputService::loadBNDFile(const std::string& fname)
	{
		if(Ogre::ResourceGroupManager::getSingleton().resourceExists(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, fname) == false)
			return false;

		Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(fname, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);

		while (!Stream->eof())
		{
			std::string Line = Stream->getLine();
			processCommand(Line);			
		}	

		return true;
	}

	//------------------------------------------------------
	void InputService::createBindContext(const std::string& ctx) 
	{
		ContextToMapper::const_iterator it = mMappers.find(ctx);

		if (it == mMappers.end()) {
			InputEventMapperPtr mapper(new InputEventMapper(this));
			mMappers.insert(make_pair(ctx, mapper));
		} else {
			LOG_ERROR("InputService::createBindContext: Context already exists: %s", ctx.c_str());
		}
	}

    //------------------------------------------------------
    bool InputService::init()
	{
    	OIS::ParamList paramList;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;

		mConfigService = GET_SERVICE(ConfigService);
        mRenderService = GET_SERVICE(RenderService);

        mRenderWindow = mRenderService->getRenderWindow();

		// Get window handle
		mRenderWindow->getCustomAttribute( "WINDOW", &windowHnd );

		// Fill parameter list
		windowHndStr << (unsigned int) windowHnd;
		paramList.insert( std::make_pair( std::string( "WINDOW" ), windowHndStr.str()));

		// Non-exclusive input - for debugging purposes
		mNonExclusive = false;

        if (mConfigService->hasParam("nonexclusive"))
            mNonExclusive = mConfigService->getParam("nonexclusive").toBool();

        if (mNonExclusive) 
		{
            #if defined OIS_WIN32_PLATFORM
            paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
            paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
            paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
            paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
            paramList.insert(std::make_pair(std::string("w32_joystick"), std::string("DISCL_FOREGROUND")));
            paramList.insert(std::make_pair(std::string("w32_joystick"), std::string("DISCL_EXCLUSIVE")));
            #elif defined OIS_LINUX_PLATFORM
            paramList.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
            paramList.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
            paramList.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
            paramList.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
            // There's no nonexclusive setting for joystick on linux...
            #endif
        }


		// Create inputsystem
		mInputSystem = OIS::InputManager::createInputSystem( paramList );

		// If possible create a buffered keyboard
#if OIS_VERSION >= 66048
		if( mInputSystem->getNumberOfDevices(OIS::OISKeyboard) > 0 ) {
#else
		if( mInputSystem->numKeyboards() > 0 ) 
		{
#endif
			mKeyboard = static_cast<OIS::Keyboard*>( mInputSystem->createInputObject( OIS::OISKeyboard, true));
			mKeyboard->setEventCallback( this );
			LOG_INFO("Found keyboard %s", mKeyboard->vendor().c_str());
		}

		// If possible create a buffered mouse
#if OIS_VERSION >= 66048
		if( mInputSystem->getNumberOfDevices(OIS::OISMouse) > 0 ) {
#else
		if( mInputSystem->numMice() > 0 ) 
		{
#endif
			mMouse = static_cast<OIS::Mouse*>( mInputSystem->createInputObject( OIS::OISMouse, true));
			mMouse->setEventCallback( this );
			LOG_INFO("Found mouse %s", mMouse->vendor().c_str());

			// Get window size
			unsigned int width, height, depth;
			int left, top;
			mRenderWindow->getMetrics( width, height, depth, left, top);

			// Set mouse region
			const OIS::MouseState &mouseState = mMouse->getMouseState();
			mouseState.width  = width;
			mouseState.height = height;
		}

#if OIS_VERSION >= 66048
		if( mInputSystem->getNumberOfDevices(OIS::OISJoyStick) > 0 ) {
#else
		if( mInputSystem->numJoySticks() > 0 ) 
		{
#endif
			mJoystick = static_cast<OIS::JoyStick*>( mInputSystem->createInputObject( OIS::OISJoyStick, true));
			mJoystick->setEventCallback( this );
			LOG_INFO("Found Joystick %s", mJoystick->vendor().c_str());
		}

		// Last step: Get the loop service and register as a listener
		mLoopService = static_pointer_cast<LoopService>(ServiceManager::getSingleton().getService("LoopService"));
		mLoopService->addLoopClient(this);

		initKeyMap();

		return true;
    }

    //------------------------------------------------------
    void InputService::bootstrapFinished()
	{
		// So the services will be able to catch up
		ServiceManager::getSingleton().createByMask(SERVICE_INPUT_LISTENER);
    }

	//------------------------------------------------------
	void InputService::shutdown() 
	{
		LOG_DEBUG("InputService::shutdown");
		mConfigService.setNull();
	}

	//------------------------------------------------------
	void InputService::loopStep(float deltaTime)
	{
		// TODO: For now. The code will move here for 0.3
		captureInputs();

		// Process the key repeat (but only for exclusive input)
		if (!mNonExclusive)
			processKeyRepeat(deltaTime/1000.0f); // loop time is in milis
	}

	//------------------------------------------------------
	void InputService::processKeyRepeat(float deltaTime) {
		if (mCurrentKey == OIS::KC_UNASSIGNED)
			return;

		mKeyPressTime += deltaTime;
		
		assert(mCurrentDelay > 0);
		assert(mRepeatDelay > 0);
		
		// see if we overlapped
		while (mKeyPressTime > mRepeatDelay) {
			mKeyPressTime -= mCurrentDelay;
			mCurrentDelay = mRepeatDelay;
			
			if (mInputMode == IM_MAPPED)
				processKeyEvent(mCurrentKey, IET_KEYBOARD_HOLD);
		}
	}

	//------------------------------------------------------
	void InputService::setBindContext(const std::string& context) 
	{
		InputEventMapperPtr iemp = findMapperForContext(context);

		if (!iemp.isNull()) 
			mCurrentMapper = iemp;
		else 
		{
			mCurrentMapper.setNull();
			LOG_ERROR("InputService::setBindContext: invalid context specified: %s", context.c_str());
		}
	}

	//------------------------------------------------------
	std::string InputService::fillVariables(const std::string& src) const 
	{
		// This is quite bad implementation actually.
		// Should use a StringTokenizer with custom rule ($ and some non-alpha like space etc split).

		WhitespaceStringTokenizer st(src);

		if (st.end())
			return src;

		string result;

		bool first = true;

		while (!st.end())
		{
			if (!first)
				result += " ";

			first = false;

			string var = st.next();

			if (var == "")
				continue;

			if (var.substr(0,1) == "$") 
			{
				ValueMap::const_iterator it = mVariables.find(var.substr(1, var.length() - 1));

				if (it != mVariables.end())
					result += it->second.toString();
				else
					result += var;

			} 
			else 
				result += var;
		}

		return result;
	}

	//------------------------------------------------------
	InputEventMapperPtr InputService::findMapperForContext(const std::string& ctx)
	{
		ContextToMapper::const_iterator it = mMappers.find(ctx);

		if (it != mMappers.end())
			return it->second;
		else
			return InputEventMapperPtr();
	}

	//------------------------------------------------------
	/// char classifier (is comment)
	struct IsComment : public std::unary_function<char, bool>
	{
		bool operator()(char c) 
		{
			return c==';';
		}
	};

	//------------------------------------------------------
	std::string InputService::stripComment(const std::string& cmd) 
	{
		IsComment isC;

		string::const_iterator it = find_if(cmd.begin(), cmd.end(), isC);

		return string(cmd.begin(), it);
	}

	//------------------------------------------------------
	DVariant InputService::getVariable(const std::string& var) 
	{
		ValueMap::const_iterator it = mVariables.find(var);

		if (it != mVariables.end())
			return it->second;
		else
			return DVariant();
	}

	//------------------------------------------------------
	void InputService::setVariable(const std::string& var, const DVariant& val)
	{
		LOG_DEBUG("InputService::setVariable: '%s' -> %s", var.c_str(), val.toString().c_str());
		ValueMap::iterator it = mVariables.find(var);

		if (it != mVariables.end()) 
			it->second = val;
		else 
			mVariables.insert(make_pair(var, val));
	}

	//------------------------------------------------------
	void InputService::captureInputs() 
	{
		if( mMouse ) 
			mMouse->capture();

		if( mKeyboard ) 
			mKeyboard->capture();

		if( mJoystick )
			mJoystick->capture();

		// now iterate through the events, for the repeated events, dispatch command
	}

	//------------------------------------------------------
	void InputService::processKeyEvent(unsigned int keyCode, InputEventType t) {
		// Some safety checks
		if (mInputMode == IM_DIRECT)
			return;
	
		if (mCurrentMapper.isNull()) {
			LOG_ERROR("InputService::processKeyEvent: Mapped input, but no mapper set for the current state!");
			return;
		}
	
		if (mKeyboard->isModifierDown(Keyboard::Alt))
			keyCode |= ALT_MOD;
		
		if (mKeyboard->isModifierDown(Keyboard::Ctrl))
			keyCode |= CTRL_MOD;
		
		if (mKeyboard->isModifierDown(Keyboard::Shift))
			keyCode |= SHIFT_MOD;
	
		std::string command;
		CommandMap::const_iterator it = mCommandMap.find(keyCode);
	
		if (it != mCommandMap.end()) {
			command = it->second;
		} else {
			LOG_DEBUG("Encountered an unmapped key event '%s'", e.text);
			return;
		}
	
		InputEventMsg msg;
		std::pair<string, string> Split = splitCommand(command);
	
		msg.command = Split.first;
		msg.params = Split.second;
		msg.event = t;
	
		if (command[0] == '+') {
			if (t == IET_KEYBOARD_PRESS)
				msg.params = 1.0f;
			else if (t == IET_KEYBOARD_RELEASE)
				msg.params = 0.0f;
			else
				// no processing for keyboard hold
				return;
	
			command = command.substr(1);
		} else if (command[0] == '-') {
			if (t == IET_KEYBOARD_PRESS)
				msg.params = 0.0f;
			else if (t == IET_KEYBOARD_RELEASE)
				msg.params = 1.0f;
			else
				// no processing for keyboard hold
				return;
			command = command.substr(1);
		} else if (t == IET_KEYBOARD_RELEASE) {
			// no key release for the non-on/off type events
			return;
		}
	
		if (!callCommandTrap(msg)) {
			LOG_DEBUG("Encountered an unmapped key event '%d'", keyCode);
		}
	}


	//------------------------------------------------------
	void InputService::processJoyMouseEvent(unsigned int id, InputEventType event)
	{
		// Some safety checks
		if (mInputMode == IM_DIRECT)
			return;

		if (mCurrentMapper.isNull()) {
			LOG_ERROR("InputService::processKeyEvent: Mapped input, but no mapper set for the current state!");
			return;
		}

		// dispatch the key event using the mapper
		unsigned int button = id;
		if((event == IET_MOUSE_PRESS) || (event == IET_MOUSE_RELEASE))
			button += DARK_MOUSE_EVENT;
		else
			button += DARK_JOY_EVENT;

		std::string command;
		CommandMap::const_iterator it = mCommandMap.find(button);
		if (it != mCommandMap.end())
			command = it->second;
		else
			return;

		InputEventMsg msg;
		std::pair<string, string> split = splitCommand(command);		

		msg.command = split.first;
		msg.params = split.second;
		msg.event = event;

		if (command[0] == '+') 
		{
			if (event == IET_MOUSE_PRESS)
				msg.params = 1.0f;
			else
				msg.params = 0.0f;
			command = command.substr(1);
		} 
		else if (command[0] == '-') 
		{
			if (event == IET_MOUSE_PRESS)
				msg.params = 0.0f;
			else
				msg.params = 1.0f;
			command = command.substr(1);
		}

		callCommandTrap(msg);
	}

	//------------------------------------------------------
	void InputService::registerCommandTrap(const std::string& command, const ListenerPtr& listener) 
	{
		if (mCommandTraps.find(command) != mCommandTraps.end()) 
		{
			// Already registered command. LOG an error
			LOG_ERROR("The command %s already has a registered trap. Not registering", command.c_str());
			return;
		} 
		else 
			mCommandTraps.insert(make_pair(command, listener));
	}

	//------------------------------------------------------
	void InputService::unregisterCommandTrap(const ListenerPtr& listener)
	{
		// Iterate through the trappers, find the ones with this listener ptr, remove
		ListenerMap::iterator it = mCommandTraps.begin();

		while (it != mCommandTraps.end()) 
		{
			ListenerMap::iterator pos = it++;

			if (pos->second == listener) 
				mCommandTraps.erase(pos);
		}
	}

	//------------------------------------------------------
	void InputService::unregisterCommandTrap(const std::string& command) 
	{
		// Iterate through the trappers, find the ones with the given command name, remove
		ListenerMap::iterator it = mCommandTraps.begin();

		while (it != mCommandTraps.end()) 
		{
			ListenerMap::iterator pos = it++;

			if (pos->first == command)
				mCommandTraps.erase(pos);
		}
	}

	//------------------------------------------------------
	bool InputService::callCommandTrap(const InputEventMsg& msg) 
	{
		ListenerMap::const_iterator it = mCommandTraps.find(msg.command);

		LOG_VERBOSE("Command trap '%s'", msg.command.c_str());
		
		if (it != mCommandTraps.end()) 
		{
			(*it->second)(msg);
			return true;
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::hasCommandTrap(const std::string& cmd) 
	{
		if (mCommandTraps.find(cmd) != mCommandTraps.end())
			return true;

		return false;
	}

	//------------------------------------------------------
	std::pair<std::string, std::string> InputService::splitCommand(const std::string& cmd) const 
	{
		WhitespaceStringTokenizer stok(cmd);

		std::pair<std::string, std::string> res = make_pair("", "");

		if (!stok.end())
			res.first = stok.next();

		if (!stok.end())
			res.second = stok.rest();

		return res;
	}

	//------------------------------------------------------
	bool InputService::keyPressed(const OIS::KeyEvent &e) 
	{
		// TODO: We could use a key filter to detect if the key is repeatable 
		mCurrentKey = e.key;
		mKeyPressTime = 0;
		mCurrentDelay = mInitialDelay;

		if (mInputMode == IM_DIRECT) {   
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->keyPressed(e);
		} else { 
			processKeyEvent(static_cast<unsigned int>(e.key), IET_KEYBOARD_PRESS);
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::keyReleased(const OIS::KeyEvent &e) 
	{
		if (e.key == mCurrentKey)
			mCurrentKey = OIS::KC_UNASSIGNED;

		
		if (mInputMode == IM_DIRECT) { 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->keyReleased(e);
		} else { 
			processKeyEvent(static_cast<unsigned int>(e.key), IET_KEYBOARD_RELEASE);
		}
		
		return false;
	}

	//------------------------------------------------------
	bool InputService::mouseMoved(const OIS::MouseEvent &e) 
	{

		if (mInputMode == IM_DIRECT) { 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->mouseMoved(e);
		} else {
			// TODO: dispatch the mouse movement using the mapper (axisx or axisy)
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		processJoyMouseEvent((int) id, IET_MOUSE_PRESS);

		if (mInputMode == IM_DIRECT) {
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->mousePressed(e, id);
		} else { 
			processJoyMouseEvent((int) id, IET_MOUSE_PRESS);
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id) 
	{

		if (mInputMode == IM_DIRECT) { 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->mouseReleased(e, id);
		} else {
			processJoyMouseEvent((int) id, IET_MOUSE_RELEASE);
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::axisMoved(const JoyStickEvent &arg, int axis)
	{
		if((arg.state.mAxes[axis].abs > JoyStick::MIN_AXIS / 10) && (arg.state.mAxes[axis].abs < JoyStick::MAX_AXIS / 10))
			return true;	//Eat the small axis movements.

		if (mInputMode == IM_DIRECT) { 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->axisMoved(arg, axis);
		} else {
			// TODO: dispatch the axis event using the mapper
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::povMoved(const OIS::JoyStickEvent &e, int pov)
	{
		if (mInputMode == IM_DIRECT) {
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->povMoved(e, pov);
		} else { 
		
			// dispatch the key event using the mapper
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::buttonPressed(const JoyStickEvent &arg, int button)
	{
		processJoyMouseEvent((int) button, IET_JOYSTICK_PRESS);

		if (mInputMode == IM_DIRECT) { 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->buttonPressed(arg, button);
		} else {
			processJoyMouseEvent((int) button, IET_JOYSTICK_PRESS);
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::buttonReleased(const JoyStickEvent &arg, int button)
	{
		if (mInputMode == IM_DIRECT) { 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->buttonReleased(arg, button);
		} else {
			processJoyMouseEvent((int) button, IET_JOYSTICK_RELEASE);
		}

		return false;
	}

    //-------------------------- Factory implementation
    std::string InputServiceFactory::mName = "InputService";

    InputServiceFactory::InputServiceFactory() : ServiceFactory()
	{
		ServiceManager::getSingleton().addServiceFactory(this);
    }

    const std::string& InputServiceFactory::getName() 
	{
		return mName;
    }

	const uint InputServiceFactory::getMask()
	{
		return SERVICE_ENGINE;
	}

    Service* InputServiceFactory::createInstance(ServiceManager* manager) 
	{
		return new InputService(manager, mName);
    }
}
