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
			mDirectListener(NULL),
			mMouse(NULL),
			mKeyboard(NULL),
			mInputMode(IM_MAPPED) {
		
    	// Loop client definition
    	mLoopClientDef.id = LOOPCLIENT_ID_INPUT;
    	mLoopClientDef.mask = LOOPMODE_INPUT;
    	mLoopClientDef.priority = LOOPCLIENT_PRIORITY_INPUT;
    	mLoopClientDef.name = mName;
    	
    	// Initialize the valid keys
		registerValidKey(KC_ESCAPE, "esc");

		registerValidKey(KC_1, "1");
		registerValidKey(KC_2, "2");
		registerValidKey(KC_3, "3");
		registerValidKey(KC_4, "4");
		registerValidKey(KC_5, "5");
		registerValidKey(KC_6, "6");
		registerValidKey(KC_7, "7");
		registerValidKey(KC_8, "8");
		registerValidKey(KC_9, "9");
		registerValidKey(KC_0, "0");

		registerValidKey(KC_MINUS, "-");
		registerValidKey(KC_EQUALS, "=");
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
		registerValidKey(KC_RBRACKET, "]");
		registerValidKey(KC_RETURN, "enter");
		registerValidKey(KC_LCONTROL, "ctrl"); // Not identified - left right?

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
		registerValidKey(KC_APOSTROPHE, "'");
		registerValidKey(KC_GRAVE, "~"); // This is also '`' it seems. Hmm. Why do they set the key twice? Are those separate on some keyboards?
		registerValidKey(KC_LSHIFT, "shift");
		registerValidKey(KC_BACKSLASH, "\\");
		registerValidKey(KC_Z, "z");
		registerValidKey(KC_X, "x");
		registerValidKey(KC_C, "c");
		registerValidKey(KC_V, "v");
		registerValidKey(KC_B, "b");
		registerValidKey(KC_N, "n");
		registerValidKey(KC_M, "m");
		registerValidKey(KC_COMMA, ",");
		registerValidKey(KC_PERIOD, ".");
		registerValidKey(KC_SLASH, "/");
		registerValidKey(KC_RSHIFT, "shift");
		registerValidKey(KC_MULTIPLY, "keypad_star"); // Keypad star?
		registerValidKey(KC_LMENU, "alt");
		registerValidKey(KC_SPACE, "space");
		// registerValidKey(KC_CAPITAL, ""); // CAPS_LOCK?
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
		// registerValidKey(KC_OEM_102, ""); // ?
		registerValidKey(KC_F11, "f11");
		registerValidKey(KC_F12, "f12");
		registerValidKey(KC_F13, "f13");
		registerValidKey(KC_F14, "f14");
		registerValidKey(KC_F15, "f15");

		// It would be nice to include these at least in some non-compatibility mode
		// registerValidKey(KC_KANA, ""); // ?
		// registerValidKey(KC_ABNT_C1, "");
		// registerValidKey(KC_CONVERT, "");
		// registerValidKey(KC_NOCONVERT, "");
		// registerValidKey(KC_YEN, "");
		// registerValidKey(KC_ABNT_C2, "");
		// registerValidKey(KC_NUMPADEQUALS, "");
		// registerValidKey(KC_PREVTRACK, "");
		// registerValidKey(KC_AT, "");
		// registerValidKey(KC_COLON, "");
		// registerValidKey(KC_UNDERLINE, "");
		// registerValidKey(KC_KANJI, "");
		// registerValidKey(KC_STOP, "");
		// registerValidKey(KC_AX, "");
		// registerValidKey(KC_UNLABELED, "");
		// registerValidKey(KC_NEXTTRACK, "");
		registerValidKey(KC_NUMPADENTER, "keypad_enter");
		// registerValidKey(KC_RCONTROL, "");
		// registerValidKey(KC_MUTE, "");
		// registerValidKey(KC_CALCULATOR, "");
		// registerValidKey(KC_PLAYPAUSE, "");
		// registerValidKey(KC_MEDIASTOP, "");
		//registerValidKey(KC_VOLUMEDOWN, "");
		// registerValidKey(KC_VOLUMEUP, "");
		// registerValidKey(KC_WEBHOME, "");
		// registerValidKey(KC_NUMPADCOMMA, "");
		// registerValidKey(KC_DIVIDE, "");
		// registerValidKey(KC_SYSRQ, "");
		registerValidKey(KC_RMENU, "alt");
		// registerValidKey(KC_PAUSE, "");
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

		// registerValidKey(KC_LWIN, "");
		// registerValidKey(KC_RWIN, "");
		// registerValidKey(KC_APPS, "");
		// registerValidKey(KC_POWER, "");
		// registerValidKey(KC_SLEEP, "");
		// registerValidKey(KC_WAKE, "");
		// registerValidKey(KC_WEBSEARCH, "");
		// registerValidKey(KC_WEBFAVORITES, "");
		// registerValidKey(KC_WEBREFRESH, "");
		// registerValidKey(KC_WEBSTOP, "");
		// registerValidKey(KC_WEBFORWARD, "");
		// registerValidKey(KC_WEBBACK, "");
		// registerValidKey(KC_MYCOMPUTER, "");
		// registerValidKey(KC_MAIL, "");
		// registerValidKey(KC_MEDIASELECT, "");

		// For simplicity, I insert the mouse / joystick events as well
		registerValidKey(KC_UNASSIGNED, "mouse_axisx");
		registerValidKey(KC_UNASSIGNED, "mouse_axisy");

		registerValidKey(KC_UNASSIGNED, "mouse1");
		registerValidKey(KC_UNASSIGNED, "mouse2");
		registerValidKey(KC_UNASSIGNED, "mouse3");

		registerValidKey(KC_UNASSIGNED, "joy_axisx");
		registerValidKey(KC_UNASSIGNED, "joy_axisy");
    }

    //------------------------------------------------------
    InputService::~InputService() {
    	mMappers.clear();
    	mCurrentMapper = NULL;

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


		if (!mLoopService.isNull())
			mLoopService->removeLoopClient(this);
			
		mRenderService.setNull();
    }

	//------------------------------------------------------
	void InputService::createBindContext(const std::string& ctx) {
		ContextToMapper::const_iterator it = mMappers.find(ctx);

		if (it == mMappers.end()) {
			InputEventMapperPtr mapper = new InputEventMapper(this);
			mMappers.insert(make_pair(ctx, mapper));
		} else {
			LOG_ERROR("InputService::createBindContext: Context already exists: %s", ctx.c_str());
		}
	}

	//------------------------------------------------------
	void InputService::registerValidKey(OIS::KeyCode kc, const std::string& txt) {
		mKeyMap.insert(make_pair(kc, txt));
		mReverseKeyMap.insert(make_pair(txt, kc));
	}

	//------------------------------------------------------
	bool InputService::isKeyTextValid(std::string& txt) {
		ReverseKeyMap::const_iterator it = mReverseKeyMap.find(txt);

		if (it != mReverseKeyMap.end())
			return true;
		else
			return false;
	}

	//------------------------------------------------------
	OIS::KeyCode InputService::getKeyTextCode(std::string& txt) {
		ReverseKeyMap::const_iterator it = mReverseKeyMap.find(txt);

		if (it != mReverseKeyMap.end())
			return it->second;
		else
			return KC_UNASSIGNED;
	}

	//------------------------------------------------------
	std::string InputService::getKeyText(OIS::KeyCode kc) {
		KeyMap::const_iterator it = mKeyMap.find(kc);

		if (it != mKeyMap.end())
			return it->second;
		else
			return "";
	}

    //------------------------------------------------------
    bool InputService::init() {
    	OIS::ParamList paramList;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;

		mConfigService = static_pointer_cast<ConfigService>(ServiceManager::getSingleton().getService("ConfigService"));
        mRenderService = static_pointer_cast<RenderService>(ServiceManager::getSingleton().getService("RenderService"));
        
        mRenderWindow = mRenderService->getRenderWindow();

		// Get window handle
		mRenderWindow->getCustomAttribute( "WINDOW", &windowHnd );

		// Fill parameter list
		windowHndStr << (unsigned int) windowHnd;
		paramList.insert( std::make_pair( std::string( "WINDOW" ), windowHndStr.str() ) );

		// Non-exclusive input - for debugging purposes
		bool nonex = false;

        if (mConfigService->hasParam("nonexclusive"))
            nonex = mConfigService->getParam("nonexclusive").toBool();

        if (nonex) {
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
        }


		// Create inputsystem
		mInputSystem = OIS::InputManager::createInputSystem( paramList );

		// If possible create a buffered keyboard
#if OIS_VERSION >= 66048
		if( mInputSystem->getNumberOfDevices(OIS::OISKeyboard) > 0 ) {
#else
		if( mInputSystem->numKeyboards() > 0 ) {
#endif
			mKeyboard = static_cast<OIS::Keyboard*>( mInputSystem->createInputObject( OIS::OISKeyboard, true ) );
			mKeyboard->setEventCallback( this );
		}

		// If possible create a buffered mouse
#if OIS_VERSION >= 66048
		if( mInputSystem->getNumberOfDevices(OIS::OISMouse) > 0 ) {
#else
		if( mInputSystem->numMice() > 0 ) {
#endif
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


		// Last step: Get the loop service and register as a listener
		mLoopService = static_pointer_cast<LoopService>(ServiceManager::getSingleton().getService("LoopService"));
		mLoopService->addLoopClient(this);
		
		return true;
    }

    //------------------------------------------------------
    void InputService::bootstrapFinished() {
		// So the services will be able to catch up
		ServiceManager::getSingleton().createByMask(SERVICE_INPUT_LISTENER);
    }

	//------------------------------------------------------
	void InputService::shutdown() {
		LOG_DEBUG("InputService::shutdown");
		mConfigService.setNull();
	}

	//------------------------------------------------------
	void InputService::loopStep(float deltaTime) {
		// TODO: For now. The code will move here for 0.3
		captureInputs();
	}
	
	//------------------------------------------------------
	void InputService::setBindContext(const std::string& context) {
		InputEventMapperPtr iemp = findMapperForContext(context);

		if (!iemp.isNull()) {
			mCurrentMapper = iemp;
		} else {
			mCurrentMapper = NULL;
			LOG_ERROR("InputService::setBindContext: invalid context specified: %s", context.c_str());
		}
	}

	//------------------------------------------------------
	DVariant InputService::command(const std::string& command) {
		LOG_DEBUG("InputService::command: '%s'", command.c_str());
		string cmd = stripComment(command);

		// Peek at first or second position. If we find bind command
		WhitespaceStringTokenizer st(cmd, false);

		if (st.end()) {
			LOG_DEBUG("InputService::command: Empty Line?");
			return false;
		}

		string f = st.next();

		if (f == "bind") {
			LOG_DEBUG("InputService::command: Bind cmd for current ctx...");

			if (!mCurrentMapper.isNull())
				return mCurrentMapper->command(cmd);
			else {
				LOG_ERROR("InputService::command: Current context is NULL! Can't bind...");
				return false;
			}

		} else if (f == "echo") {
			LOG_DEBUG("InputService::command: Echo cmd");

			if (st.end())
				return DVariant();

			string result;
			bool first = true;

			while (!st.end()) {
				if (!first)
					result += " ";

				first = false;

				string var = st.next();

				if (var == "")
					continue;

				if (var.substr(1,1) == "$") {
					ValueMap::const_iterator it = mVariables.find(st.next());

					if (it != mVariables.end())
						result += it->second.toString();

				} else {
					result += var;
				}
			}
		} /*else if (commandDefined(f)) {
			string f1 = "";

			if (!st.end())
				f1 = st.rest();

			callCommandTrap(f, f1);

			return true;
		}
*/
		if (st.end()) {
			LOG_DEBUG("InputService::command: incomplete command?");
			return false;
		}

		string f1 = st.next();

		if (f1 == "bind") {
			LOG_DEBUG("InputService::command: bind for context %s", f.c_str());
			InputEventMapperPtr imp = findMapperForContext(f);

			if (!imp.isNull())
				return imp->command(f1);
			else
				LOG_ERROR("Unknown context for binding operation : %s", f.c_str());
		}

		// Default process: set variable value
		setVariable(f, f1);

		return false;
	}

	//------------------------------------------------------
	bool InputService::validateEventString(const std::string& ev) {
		IsChar isplus('+');

		StringTokenizer stc(ev, isplus, false);

		while (!stc.end()) {
			string key = stc.next();

			LOG_DEBUG("InputService::validateEventString: Validating key part: %s", key.c_str());

			// search for the definition of the given key. if not found, return false
			if (!isKeyTextValid(key)) {
				LOG_ERROR("InputService::validateEventString: Encountered an invalid key code: %s", key.c_str());
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------
	InputEventMapperPtr InputService::findMapperForContext(const std::string& ctx) {
		ContextToMapper::const_iterator it = mMappers.find(ctx);

		if (it != mMappers.end())
			return it->second;
		else
			return NULL;

	}

	//------------------------------------------------------
	/// char classifier (is comment)
	struct IsComment : public std::unary_function<char, bool> {
		bool operator()(char c) {
			return c==';';
		}
	};

	//------------------------------------------------------
	std::string InputService::stripComment(const std::string& cmd) {
		IsComment isC;

		string::const_iterator it = find_if(cmd.begin(), cmd.end(), isC);

		return string(cmd.begin(), it);
	}

	//------------------------------------------------------
	void InputService::loadBNDFile(const std::string& filename) {
		// open as ogre::data_stream, read line by line, execute as command
		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);

		//TODO: To be completely glad, I'd have to write a line reading functionality in File class. not there now, I'm afraid

		while (!stream->eof()) {
			std::string line = stream->getLine();

			// send to the command processing
			command(line);
		}
	}


	//------------------------------------------------------
	DVariant InputService::getVariable(const std::string& var) {
		ValueMap::const_iterator it = mVariables.find(var);

		if (it != mVariables.end()) {
			return it->second;
		} else
			return DVariant();
	}

	//------------------------------------------------------
	void InputService::setVariable(const std::string& var, const DVariant& val) {
		LOG_DEBUG("InputService::setVariable: '%s' -> %s", var.c_str(), val.toString().c_str());
		ValueMap::iterator it = mVariables.find(var);

		if (it != mVariables.end()) {
			it->second = val;
		} else {
			mVariables.insert(make_pair(var, val));
		}
	}


	//------------------------------------------------------
	void InputService::attachModifiers(std::string& tgt) {
		// Order is cruical here. The bindings also use this order (and it is ALT, CTRL, SHIFT)
		if (mKeyboard->isModifierDown(Keyboard::Alt)) {
			tgt += "+" + getKeyText(KC_LMENU);
		}

		if (mKeyboard->isModifierDown(Keyboard::Ctrl)) {
			tgt += "+" + getKeyText(KC_LCONTROL);
		}

		if (mKeyboard->isModifierDown(Keyboard::Shift)) {
			tgt += "+" + getKeyText(KC_LSHIFT);
		}
	}

	//------------------------------------------------------
	void InputService::captureInputs() {
		if( mMouse ) {
			mMouse->capture();
		}

		if( mKeyboard ) {
			mKeyboard->capture();
		}

		// now iterate through the events, for the repeated events, dispatch command
	}

	//------------------------------------------------------
	void InputService::processKeyEvent(const OIS::KeyEvent &e, InputEventType t) {
		// Some safety checks
		if (mInputMode == IM_DIRECT)
			return;

		if (mCurrentMapper.isNull()) {
			LOG_ERROR("InputService::processKeyEvent: Mapped input, but no mapper set for the current state!");
			return;
		}

		// dispatch the key event using the mapper
		string key = getKeyText(e.key);

		InputEventMapper::Command result;

		// first cycle without mods, second with them
		for (int it = 0; it < 2; it++) { // Just to run this two times
			
			// Try to find the key without the modifiers
			if (mCurrentMapper->unmapEvent(key, result)) {
				// TODO: Send the event
				// Split the command to cmd and params parts
				std::pair<string, string> split = splitCommand(result.command);
				
				InputEventMsg msg;
				
				msg.command = split.first;
				msg.params = split.second;
				msg.event = t;
				
				// If it is denoted by plus sign, we replace the param with on/off 1.0/0.0 (- does the opposite, really)
				if (result.type == InputEventMapper::CET_PEDGE) { 
					if (t == IET_KEYBOARD_PRESS)
						msg.params = 1.0f;
					else
						msg.params = 0.0f;
				} else if (result.type == InputEventMapper::CET_NEDGE) {
					if (t == IET_KEYBOARD_PRESS)
						msg.params = 0.0f;
					else
						msg.params = 1.0f;
				}
				
				// No key repeat for now...
				// If it would be CET_NORMAL. We could mark the event as running, then process every frame/key repeat
				// Don't worry. It will definatelly be implemented.
				
				// If the binding is not +/-, only progress on button press. This will be handled differently with the key repeats
				if (t != IET_KEYBOARD_RELEASE && result.type == InputEventMapper::CET_NORMAL)
					if (callCommandTrap(msg))
						return;
			}

			// no event for the key itself, try the modifiers
			attachModifiers(key);
		}
		
	}

	//------------------------------------------------------	
	void InputService::registerCommandTrap(const std::string& command, const ListenerPtr& listener) {
		if (mCommandTraps.find(command) != mCommandTraps.end()) {
			// Already registered command. LOG an error
			LOG_ERROR("The command %s already has a registered trap. Not registering", command.c_str());
			return;
		} else {
			mCommandTraps.insert(make_pair(command, listener));
		}
	}

	//------------------------------------------------------	
	void InputService::unregisterCommandTrap(const ListenerPtr& listener) {
		// Iterate through the trappers, find the ones with this listener ptr, remove
		ListenerMap::iterator it = mCommandTraps.begin();
		
		while (it != mCommandTraps.end()) {
			ListenerMap::iterator pos = it++;
			
			if (pos->second == listener) {
				mCommandTraps.erase(pos);
			}
		}
	}
	
	//------------------------------------------------------	
	void InputService::unregisterCommandTrap(const std::string& command) {
		// Iterate through the trappers, find the ones with the given command name, remove
		ListenerMap::iterator it = mCommandTraps.begin();
		
		while (it != mCommandTraps.end()) {
			ListenerMap::iterator pos = it++;
			
			if (pos->first == command) {
				mCommandTraps.erase(pos);
			}
		}
	}
	
	//------------------------------------------------------	
	bool InputService::callCommandTrap(const InputEventMsg& msg) {
		ListenerMap::const_iterator it = mCommandTraps.find(msg.command);
		
		if (it != mCommandTraps.end()) {
			(*it->second)(msg);
			return true;
		}
		
		return false;
	}

	//------------------------------------------------------
	bool InputService::hasCommandTrap(const std::string& cmd) {
		if (mCommandTraps.find(cmd) != mCommandTraps.end()) {
			return true;
		}
		
		return false;
	}

	//------------------------------------------------------
	std::pair<std::string, std::string> InputService::splitCommand(const std::string& cmd) const {
		WhitespaceStringTokenizer stok(cmd);
		
		std::pair<std::string, std::string> res = make_pair("", "");
		
		if (!stok.end())
			res.first = stok.next();
		
		if (!stok.end())
			res.second = stok.rest();
			
		return res;
	}

	//------------------------------------------------------
	bool InputService::keyPressed( const OIS::KeyEvent &e ) {

		if (mInputMode == IM_DIRECT) { // direct event, dispatch to the current listener
			if (mDirectListener)
				return mDirectListener->keyPressed(e);
		} else {
			processKeyEvent(e, IET_KEYBOARD_PRESS);
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::keyReleased( const OIS::KeyEvent &e ) {

		if (mInputMode == IM_DIRECT) { // direct event, dispatch to the current listener
			if (mDirectListener)
				return mDirectListener->keyReleased(e);
		} else {
			// dispatch the key event using the mapper
			processKeyEvent(e, IET_KEYBOARD_RELEASE);
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::mouseMoved( const OIS::MouseEvent &e ) {

		if (mInputMode == IM_DIRECT) { // direct event, dispatch to the current listener
			if (mDirectListener)
				return mDirectListener->mouseMoved(e);
		} else {
			// dispatch the key event using the mapper
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {

		if (mInputMode == IM_DIRECT) { // direct event, dispatch to the current listener
			if (mDirectListener)
				return mDirectListener->mousePressed(e, id);
		} else {
			// dispatch the key event using the mapper
		}

		return false;
	}

	//------------------------------------------------------
	bool InputService::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {

		if (mInputMode == IM_DIRECT) { // direct event, dispatch to the current listener
			if (mDirectListener)
				return mDirectListener->mouseReleased(e, id);
		} else {
			// dispatch the key event using the mapper
		}

		return false;
	}


    //-------------------------- Factory implementation
    std::string InputServiceFactory::mName = "InputService";

    InputServiceFactory::InputServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
    };

    const std::string& InputServiceFactory::getName() {
		return mName;
    }

	const uint InputServiceFactory::getMask() {
		return SERVICE_ENGINE;
	}

    Service* InputServiceFactory::createInstance(ServiceManager* manager) {
	return new InputService(manager, mName);
    }

}
