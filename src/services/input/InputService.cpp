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

		mCurrentMapper = new InputEventMapper(this);
    }

    //------------------------------------------------------
    InputService::~InputService() 
	{
    	mMappers.clear();
    	mCurrentMapper = NULL;

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
	void InputService::RegisterValidKey(int kc, const std::string& txt) 
	{
		mKeyMap.insert(make_pair(kc, txt));
		mReverseKeyMap.insert(make_pair(txt, kc));
	}

	//------------------------------------------------------
	void InputService::InitKeyMap()
	{
		RegisterValidKey(KC_ESCAPE, "esc");

		RegisterValidKey(KC_1, "1");
		RegisterValidKey(KC_1 | SHIFT_MOD, "!");
		RegisterValidKey(KC_2, "2");
		RegisterValidKey(KC_2 | SHIFT_MOD, "@");
		RegisterValidKey(KC_3, "3");
		RegisterValidKey(KC_3 | SHIFT_MOD, "#");
		RegisterValidKey(KC_4, "4");
		RegisterValidKey(KC_4 | SHIFT_MOD, "$");
		RegisterValidKey(KC_5, "5");
		RegisterValidKey(KC_5 | SHIFT_MOD, "%");
		RegisterValidKey(KC_6, "6");
		RegisterValidKey(KC_6 | SHIFT_MOD, "^");
		RegisterValidKey(KC_7, "7");
		RegisterValidKey(KC_7 | SHIFT_MOD, "&");
		RegisterValidKey(KC_8, "8");
		RegisterValidKey(KC_8 | SHIFT_MOD, "*");
		RegisterValidKey(KC_9, "9");
		RegisterValidKey(KC_9 | SHIFT_MOD, "(");
		RegisterValidKey(KC_0, "0");
		RegisterValidKey(KC_0 | SHIFT_MOD, ")");

		RegisterValidKey(KC_MINUS, "-");
		RegisterValidKey(KC_MINUS | SHIFT_MOD, "_");
		RegisterValidKey(KC_EQUALS, "=");
		RegisterValidKey(KC_EQUALS | SHIFT_MOD, "+");
		RegisterValidKey(KC_BACK, "backspace");
		RegisterValidKey(KC_TAB, "tab");

		RegisterValidKey(KC_Q, "q");
		RegisterValidKey(KC_W, "w");
		RegisterValidKey(KC_E, "e");
		RegisterValidKey(KC_R, "r");
		RegisterValidKey(KC_T, "t");
		RegisterValidKey(KC_Y, "y");
		RegisterValidKey(KC_U, "u");
		RegisterValidKey(KC_I, "i");
		RegisterValidKey(KC_O, "o");
		RegisterValidKey(KC_P, "p");

		RegisterValidKey(KC_LBRACKET, "[");
		RegisterValidKey(KC_LBRACKET | SHIFT_MOD, "{");
		RegisterValidKey(KC_RBRACKET, "]");
		RegisterValidKey(KC_RBRACKET | SHIFT_MOD, "}");
		RegisterValidKey(KC_RETURN, "enter");
		RegisterValidKey(KC_LCONTROL, "ctrl");

		RegisterValidKey(KC_A, "a");
		RegisterValidKey(KC_S, "s");
		RegisterValidKey(KC_D, "d");
		RegisterValidKey(KC_F, "f");
		RegisterValidKey(KC_G, "g");
		RegisterValidKey(KC_H, "h");
		RegisterValidKey(KC_J, "j");
		RegisterValidKey(KC_K, "k");
		RegisterValidKey(KC_L, "l");
		RegisterValidKey(KC_SEMICOLON, ";");
		RegisterValidKey(KC_SEMICOLON | SHIFT_MOD, ":");
		RegisterValidKey(KC_APOSTROPHE, "'");
		RegisterValidKey(KC_APOSTROPHE | SHIFT_MOD, "\"");
		RegisterValidKey(KC_LSHIFT, "shift");
		RegisterValidKey(KC_BACKSLASH, "\\");
		RegisterValidKey(KC_BACKSLASH | SHIFT_MOD, "|");
		RegisterValidKey(KC_Z, "z");
		RegisterValidKey(KC_X, "x");
		RegisterValidKey(KC_C, "c");
		RegisterValidKey(KC_V, "v");
		RegisterValidKey(KC_B, "b");
		RegisterValidKey(KC_N, "n");
		RegisterValidKey(KC_M, "m");
		RegisterValidKey(KC_COMMA, ",");
		RegisterValidKey(KC_COMMA | SHIFT_MOD, "<");
		RegisterValidKey(KC_PERIOD, ".");
		RegisterValidKey(KC_PERIOD | SHIFT_MOD, ">");
		RegisterValidKey(KC_SLASH, "/");
		RegisterValidKey(KC_SLASH, "keypad_slash");
		RegisterValidKey(KC_SLASH | SHIFT_MOD, "?");
		RegisterValidKey(KC_RSHIFT, "shift");
		RegisterValidKey(KC_MULTIPLY, "keypad_star");
		RegisterValidKey(KC_LMENU, "alt");
		RegisterValidKey(KC_SPACE, "space");
		RegisterValidKey(KC_F1, "f1");
		RegisterValidKey(KC_F2, "f2");
		RegisterValidKey(KC_F3, "f3");
		RegisterValidKey(KC_F4, "f4");
		RegisterValidKey(KC_F5, "f5");
		RegisterValidKey(KC_F6, "f6");
		RegisterValidKey(KC_F7, "f7");
		RegisterValidKey(KC_F8, "f8");
		RegisterValidKey(KC_F9, "f9");
		RegisterValidKey(KC_F10, "f10");
		RegisterValidKey(KC_NUMLOCK, "numlock");
		RegisterValidKey(KC_SCROLL, "scroll");
		RegisterValidKey(KC_NUMPAD7, "keypad_home");
		RegisterValidKey(KC_NUMPAD8, "keypad_up");
		RegisterValidKey(KC_NUMPAD9, "keypad_pgup");
		RegisterValidKey(KC_SUBTRACT, "keypad_minus");
		RegisterValidKey(KC_NUMPAD4, "keypad_left");
		RegisterValidKey(KC_NUMPAD5, "keypad_center");
		RegisterValidKey(KC_NUMPAD6, "keypad_right");
		RegisterValidKey(KC_ADD, "keypad_plus");
		RegisterValidKey(KC_NUMPAD1, "keypad_end");
		RegisterValidKey(KC_NUMPAD2, "keypad_down");
		RegisterValidKey(KC_NUMPAD3, "keypad_pgdn");
		RegisterValidKey(KC_NUMPAD0, "keypad_ins");
		RegisterValidKey(KC_DECIMAL, "keypad_del");
		RegisterValidKey(KC_F11, "f11");
		RegisterValidKey(KC_F12, "f12");
		RegisterValidKey(KC_F13, "f13");
		RegisterValidKey(KC_F14, "f14");
		RegisterValidKey(KC_F15, "f15");

		RegisterValidKey(KC_NUMPADENTER, "keypad_enter");
		RegisterValidKey(KC_RMENU, "alt");
		RegisterValidKey(KC_HOME, "home");
		RegisterValidKey(KC_UP, "up");
		RegisterValidKey(KC_PGUP, "pgup");
		RegisterValidKey(KC_LEFT, "left");
		RegisterValidKey(KC_RIGHT, "right");
		RegisterValidKey(KC_END, "end");
		RegisterValidKey(KC_DOWN, "down");
		RegisterValidKey(KC_PGDOWN, "pgdn");
		RegisterValidKey(KC_INSERT, "ins");
		RegisterValidKey(KC_DELETE, "del");
		RegisterValidKey(KC_GRAVE, "`");
		RegisterValidKey(KC_GRAVE | SHIFT_MOD, "~");
		RegisterValidKey(KC_SYSRQ, "print_screen");
		RegisterValidKey(joy_axisr, "joy_axisr");
		RegisterValidKey(joy_axisx, "joy_axisx");
		RegisterValidKey(joy_axisy, "joy_axisy");
		RegisterValidKey(joy_hat_up, "joy_hatup");
		RegisterValidKey(joy_hat_down, "joy_hatdn");
		RegisterValidKey(joy_hat_right, "joy_hatrt");
		RegisterValidKey(joy_hat_left, "joy_hatlt");
		RegisterValidKey(joy_1, "joy1");
		RegisterValidKey(joy_2, "joy2");
		RegisterValidKey(joy_3, "joy3");
		RegisterValidKey(joy_4, "joy4");
		RegisterValidKey(joy_5, "joy5");
		RegisterValidKey(joy_6, "joy6");
		RegisterValidKey(joy_7, "joy7");
		RegisterValidKey(joy_8, "joy8");
		RegisterValidKey(joy_9, "joy9");
		RegisterValidKey(joy_10, "joy10");
		RegisterValidKey(Mouse1, "mouse1");
		RegisterValidKey(Mouse2, "mouse2");
		RegisterValidKey(Mouse3, "mouse3");
		RegisterValidKey(Mouse4, "mouse4");
		RegisterValidKey(Mouse5, "mouse5");
		RegisterValidKey(Mouse6, "mouse6");
		RegisterValidKey(Mouse7, "mouse7");
		RegisterValidKey(Mouse8, "mouse8");
		RegisterValidKey(Mouse_axisx, "mouse_axisx");
		RegisterValidKey(Mouse_axisy, "mouse_axisy");
		RegisterValidKey(Mouse_wheel, "mouse_wheel");
	}

	//------------------------------------------------------
	void InputService::Tokenize(std::string InString, std::vector<std::string> &OutVector, char Token)
	{
		unsigned int OldOffset = 0, NewOffset;

		do
		{
			NewOffset = InString.find(Token, OldOffset);
			if((NewOffset == string::npos) || (InString.at(OldOffset) == 0x22))
				NewOffset = InString.length();
			OutVector.push_back (InString.substr(OldOffset, NewOffset - OldOffset));
			OldOffset = NewOffset + 1;
		}
		while(OldOffset < InString.length());
	}

	//------------------------------------------------------
	unsigned int InputService::MapToOISCode(std::string Key) const
	{
		unsigned int Code;
		ReverseKeyMap::const_iterator it = mReverseKeyMap.find(Key);

		if (it != mReverseKeyMap.end())
			Code = it->second;
		else
			Code = KC_UNASSIGNED;	//If we get here, then there is a key name that DarkEngine creates that we didn't cover

		return Code;
	}

	//------------------------------------------------------
	void InputService::AddBindCommand(ContentsVector Command)
	{
		ContentsVector Contents;
		unsigned int Modifier = 0;
		std::string Key, Keys = Command.at(1);

		if((Keys.length() > 1) && (Keys.find('+') != string::npos))
		{
			Contents.clear();
			Tokenize(Keys, Contents, '+');
			for(ContentsVector::const_iterator Content = Contents.begin(); Content != Contents.end(); Content++)
			{
				std::string ContentStr = *Content;
				if (ContentStr == "shift")
					Modifier |= SHIFT_MOD;
				else if (ContentStr == "ctrl")
					Modifier |= CTRL_MOD;
				else if (ContentStr == "alt")
					Modifier |= ALT_MOD;
				else
					Key = ContentStr;
			}
		}
		else
			Key = Keys;
		
		const unsigned int Code = MapToOISCode(Key);
		CommandMap.insert(make_pair(Code | Modifier, stripComment(Command.at(2))));
	}

	//------------------------------------------------------
	DVariant InputService::AddCommand(std::string& CommandString)
	{
		ContentsVector Contents, Command;

		if((CommandString.length() == 0) || (CommandString.at(0) == ';'))
			return false;
		std::transform(CommandString.begin(), CommandString.end(), CommandString.begin(), ::tolower);	//Lowercase
		Tokenize(CommandString, Command, ' ');
		if(Command.at(0) == "bind")
		{
			AddBindCommand(Command);
			return true;
		}
		else if(Command.at(0) == "echo")
			return CommandString.substr(5);
		else if(Command.at(0) == "set")
		{
			setVariable(Command.at(1), Command.at(2));
			return true;
		}
		
		setVariable(Command.at(0), Command.at(1));
		return true;
	}

	//------------------------------------------------------
	bool InputService::LoadBNDFile(const std::string& FileName)
	{
		if(Ogre::ResourceGroupManager::getSingleton().resourceExists(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, FileName) == false)
			return false;

		Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(FileName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);

		while (!Stream->eof())
		{
			std::string Line = Stream->getLine();
			AddCommand(Line);			
		}	

		return true;
	}

	//------------------------------------------------------
	void InputService::createBindContext(const std::string& ctx) 
	{
		ContextToMapper::const_iterator it = mMappers.find(ctx);

		if (it == mMappers.end()) {
			InputEventMapperPtr mapper = new InputEventMapper(this);
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
		bool nonex = false;

        if (mConfigService->hasParam("nonexclusive"))
            nonex = mConfigService->getParam("nonexclusive").toBool();

        if (nonex) 
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

		InitKeyMap();
		if(LoadBNDFile("user.bnd") == false)
			LoadBNDFile("dark.bnd");

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
	}

	//------------------------------------------------------
	void InputService::setBindContext(const std::string& context) 
	{
		InputEventMapperPtr iemp = findMapperForContext(context);

		if (!iemp.isNull()) 
			mCurrentMapper = iemp;
		else 
		{
			mCurrentMapper = NULL;
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
			return NULL;
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
	void InputService::ProcessKeyEvent(const OIS::KeyEvent &e, InputEventType t)
	{
		// Some safety checks
		if (mInputMode == IM_DIRECT)
			return;

		if (mCurrentMapper.isNull()) {
			LOG_ERROR("InputService::processKeyEvent: Mapped input, but no mapper set for the current state!");
			return;
		}

		// dispatch the key event using the mapper
		unsigned int Key = (unsigned int)e.key;

		if (mKeyboard->isModifierDown(Keyboard::Alt))
			Key |= ALT_MOD;
		if (mKeyboard->isModifierDown(Keyboard::Ctrl))
			Key |= CTRL_MOD;
		if (mKeyboard->isModifierDown(Keyboard::Shift))
			Key |= SHIFT_MOD;

		std::string Command;
		CommandMapVector::const_iterator it = CommandMap.find(Key);
		if (it != CommandMap.end())
			Command = it->second;
		else
			return;

		InputEventMsg Msg;
		std::pair<string, string> Split = splitCommand(Command);		

		Msg.command = Split.first;
		Msg.params = Split.second;
		Msg.event = t;

		if (Command[0] == '+') 
		{
			if (t == IET_KEYBOARD_PRESS)
				Msg.params = 1.0f;
			else
				Msg.params = 0.0f;
			Command = Command.substr(1);
		} 
		else if (Command[0] == '-') 
		{
			if (t == IET_KEYBOARD_PRESS)
				Msg.params = 0.0f;
			else
				Msg.params = 1.0f;
			Command = Command.substr(1);
		}

		callCommandTrap(Msg);
	}

	//------------------------------------------------------
	void InputService::ProcessJoyMouseEvent(unsigned int Id, InputEventType Event)
	{
		// Some safety checks
		if (mInputMode == IM_DIRECT)
			return;

		if (mCurrentMapper.isNull()) {
			LOG_ERROR("InputService::processKeyEvent: Mapped input, but no mapper set for the current state!");
			return;
		}

		// dispatch the key event using the mapper
		unsigned int Button = Id;
		if((Event == IET_MOUSE_PRESS) || (Event == IET_MOUSE_RELEASE))
			Button += DARK_MOUSE_EVENT;
		else
			Button += DARK_JOY_EVENT;

		std::string Command;
		CommandMapVector::const_iterator it = CommandMap.find(Button);
		if (it != CommandMap.end())
			Command = it->second;
		else
			return;

		InputEventMsg Msg;
		std::pair<string, string> Split = splitCommand(Command);		

		Msg.command = Split.first;
		Msg.params = Split.second;
		Msg.event = Event;

		if (Command[0] == '+') 
		{
			if (Event == IET_MOUSE_PRESS)
				Msg.params = 1.0f;
			else
				Msg.params = 0.0f;
			Command = Command.substr(1);
		} 
		else if (Command[0] == '-') 
		{
			if (Event == IET_MOUSE_PRESS)
				Msg.params = 0.0f;
			else
				Msg.params = 1.0f;
			Command = Command.substr(1);
		}

		callCommandTrap(Msg);
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

		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->keyPressed(e);
		else 
			ProcessKeyEvent(e, IET_KEYBOARD_PRESS);

		return false;
	}

	//------------------------------------------------------
	bool InputService::keyReleased(const OIS::KeyEvent &e) 
	{

		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->keyReleased(e);
		else 
			ProcessKeyEvent(e, IET_KEYBOARD_RELEASE);

		return false;
	}

	//------------------------------------------------------
	bool InputService::mouseMoved(const OIS::MouseEvent &e) 
	{

		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->mouseMoved(e);
		else 
		{
			// dispatch the key event using the mapper
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		ProcessJoyMouseEvent((int) id, IET_MOUSE_PRESS);

		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->mousePressed(e, id);
		else 
			ProcessJoyMouseEvent((int) id, IET_MOUSE_PRESS);

		return false;
	}

	//------------------------------------------------------
	bool InputService::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id) 
	{

		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->mouseReleased(e, id);
		else
			ProcessJoyMouseEvent((int) id, IET_MOUSE_RELEASE);

		return false;
	}

	//------------------------------------------------------
	bool InputService::axisMoved(const JoyStickEvent &arg, int axis)
	{
		if((arg.state.mAxes[axis].abs > JoyStick::MIN_AXIS / 10) && (arg.state.mAxes[axis].abs < JoyStick::MAX_AXIS / 10))
			return true;	//Eat the small axis movements.

		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->axisMoved(arg, axis);
		else
		{
			// dispatch the key event using the mapper
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::povMoved(const OIS::JoyStickEvent &e, int pov)
	{
		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->povMoved(e, pov);
		else 
		{
			// dispatch the key event using the mapper
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::buttonPressed(const JoyStickEvent &arg, int button)
	{
		ProcessJoyMouseEvent((int) button, IET_JOYSTICK_PRESS);

		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->buttonPressed(arg, button);
		else
			ProcessJoyMouseEvent((int) button, IET_JOYSTICK_PRESS);

		return false;
	}

	//------------------------------------------------------
	bool InputService::buttonReleased(const JoyStickEvent &arg, int button)
	{
		if (mInputMode == IM_DIRECT) 
			if (mDirectListener)	// direct event, dispatch to the current listener
				return mDirectListener->buttonReleased(arg, button);
		else
			ProcessJoyMouseEvent((int) button, IET_JOYSTICK_RELEASE);

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
