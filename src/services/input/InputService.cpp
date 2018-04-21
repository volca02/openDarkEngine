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

#include "InputService.h"
#include "OpdeException.h"
#include "StringTokenizer.h"
#include "config/ConfigService.h"
#include "logger.h"
#include "loop/LoopService.h"
#include "render/RenderService.h"

using namespace std;
using namespace Ogre;

namespace Opde {

/*-----------------------------------------------------*/
/*-------------------- InputService -------------------*/
/*-----------------------------------------------------*/
template <> const size_t ServiceImpl<InputService>::SID = __SERVICE_ID_INPUT;

InputService::InputService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<Opde::InputService>(manager, name), mInputMode(IM_MAPPED),
      mDirectListener(NULL),
      mInitialDelay(0.4f), // TODO: Read these from the config service
      mRepeatDelay(0.3f), mKeyPressTime(0.0f), mCurrentKey(SDLK_UNKNOWN),
      mCurrentMods(0), mNonExclusive(false) {
    // Loop client definition
    mLoopClientDef.id = LOOPCLIENT_ID_INPUT;
    mLoopClientDef.mask = LOOPMODE_INPUT;
    mLoopClientDef.priority = LOOPCLIENT_PRIORITY_INPUT;
    mLoopClientDef.name = mName;

    mDefaultMapper = new InputEventMapper(this, "-DEFAULT-");
    mCurrentMapper = mDefaultMapper;
}

//------------------------------------------------------
InputService::~InputService() {
    ContextToMapper::iterator it = mMappers.begin();
    for (; it != mMappers.end(); ++it) {
        delete it->second;
        it->second = NULL;
    }

    mMappers.clear();
    delete mDefaultMapper;
    mDefaultMapper = NULL;
    mCurrentMapper = NULL;

    if (mLoopService)
        mLoopService->removeLoopClient(this);

    mRenderService.reset();
}

//------------------------------------------------------
void InputService::registerValidKey(int kc, const std::string &txt) {
    mKeyMap.insert(make_pair(kc, txt));
    mReverseKeyMap.insert(make_pair(txt, kc));
}

//------------------------------------------------------
void InputService::initKeyMap() {
    registerValidKey(SDLK_ESCAPE, "esc");

    registerValidKey(SDLK_1, "1");
    registerValidKey(SDLK_1 | SHIFT_MOD, "!");
    registerValidKey(SDLK_2, "2");
    registerValidKey(SDLK_2 | SHIFT_MOD, "@");
    registerValidKey(SDLK_3, "3");
    registerValidKey(SDLK_3 | SHIFT_MOD, "#");
    registerValidKey(SDLK_4, "4");
    registerValidKey(SDLK_4 | SHIFT_MOD, "$");
    registerValidKey(SDLK_5, "5");
    registerValidKey(SDLK_5 | SHIFT_MOD, "%");
    registerValidKey(SDLK_6, "6");
    registerValidKey(SDLK_6 | SHIFT_MOD, "^");
    registerValidKey(SDLK_7, "7");
    registerValidKey(SDLK_7 | SHIFT_MOD, "&");
    registerValidKey(SDLK_8, "8");
    registerValidKey(SDLK_8 | SHIFT_MOD, "*");
    registerValidKey(SDLK_9, "9");
    registerValidKey(SDLK_9 | SHIFT_MOD, "(");
    registerValidKey(SDLK_0, "0");
    registerValidKey(SDLK_0 | SHIFT_MOD, ")");

    registerValidKey(SDLK_MINUS, "-");
    registerValidKey(SDLK_MINUS | SHIFT_MOD, "_");
    registerValidKey(SDLK_EQUALS, "=");
    registerValidKey(SDLK_EQUALS | SHIFT_MOD, "+");
    registerValidKey(SDLK_BACKSPACE, "backspace");
    registerValidKey(SDLK_TAB, "tab");

    registerValidKey(SDLK_q, "q");
    registerValidKey(SDLK_w, "w");
    registerValidKey(SDLK_e, "e");
    registerValidKey(SDLK_r, "r");
    registerValidKey(SDLK_t, "t");
    registerValidKey(SDLK_y, "y");
    registerValidKey(SDLK_u, "u");
    registerValidKey(SDLK_i, "i");
    registerValidKey(SDLK_o, "o");
    registerValidKey(SDLK_p, "p");

    registerValidKey(SDLK_LEFTBRACKET, "[");
    registerValidKey(SDLK_LEFTBRACKET | SHIFT_MOD, "{");
    registerValidKey(SDLK_RIGHTBRACKET, "]");
    registerValidKey(SDLK_RIGHTBRACKET | SHIFT_MOD, "}");
    registerValidKey(SDLK_RETURN, "enter");
    registerValidKey(SDLK_LCTRL, "ctrl");

    registerValidKey(SDLK_a, "a");
    registerValidKey(SDLK_s, "s");
    registerValidKey(SDLK_d, "d");
    registerValidKey(SDLK_f, "f");
    registerValidKey(SDLK_g, "g");
    registerValidKey(SDLK_h, "h");
    registerValidKey(SDLK_j, "j");
    registerValidKey(SDLK_k, "k");
    registerValidKey(SDLK_l, "l");
    registerValidKey(SDLK_SEMICOLON, ";");
    registerValidKey(SDLK_SEMICOLON | SHIFT_MOD, ":");
    registerValidKey(SDLK_QUOTE, "'");
    registerValidKey(SDLK_QUOTE | SHIFT_MOD, "\"");
    registerValidKey(SDLK_LSHIFT, "shift");
    registerValidKey(SDLK_BACKSLASH, "\\");
    registerValidKey(SDLK_BACKSLASH | SHIFT_MOD, "|");
    registerValidKey(SDLK_z, "z");
    registerValidKey(SDLK_x, "x");
    registerValidKey(SDLK_c, "c");
    registerValidKey(SDLK_v, "v");
    registerValidKey(SDLK_b, "b");
    registerValidKey(SDLK_n, "n");
    registerValidKey(SDLK_m, "m");
    registerValidKey(SDLK_COMMA, ",");
    registerValidKey(SDLK_COMMA | SHIFT_MOD, "<");
    registerValidKey(SDLK_PERIOD, ".");
    registerValidKey(SDLK_PERIOD | SHIFT_MOD, ">");
    registerValidKey(SDLK_SLASH, "/");
    registerValidKey(SDLK_SLASH, "keypad_slash");
    registerValidKey(SDLK_SLASH | SHIFT_MOD, "?");
    registerValidKey(SDLK_RSHIFT, "shift");
    registerValidKey(SDLK_ASTERISK, "keypad_star");
    registerValidKey(SDLK_LALT, "alt");
    registerValidKey(SDLK_SPACE, "space");
    registerValidKey(SDLK_F1, "f1");
    registerValidKey(SDLK_F2, "f2");
    registerValidKey(SDLK_F3, "f3");
    registerValidKey(SDLK_F4, "f4");
    registerValidKey(SDLK_F5, "f5");
    registerValidKey(SDLK_F6, "f6");
    registerValidKey(SDLK_F7, "f7");
    registerValidKey(SDLK_F8, "f8");
    registerValidKey(SDLK_F9, "f9");
    registerValidKey(SDLK_F10, "f10");
    registerValidKey(SDLK_NUMLOCKCLEAR, "numlock");
    registerValidKey(SDLK_SCROLLLOCK, "scroll");
    registerValidKey(SDLK_KP_7, "keypad_home");
    registerValidKey(SDLK_KP_8, "keypad_up");
    registerValidKey(SDLK_KP_9, "keypad_pgup");
    registerValidKey(SDLK_KP_MINUS, "keypad_minus");
    registerValidKey(SDLK_KP_4, "keypad_left");
    registerValidKey(SDLK_KP_5, "keypad_center");
    registerValidKey(SDLK_KP_6, "keypad_right");
    registerValidKey(SDLK_KP_PLUS, "keypad_plus");
    registerValidKey(SDLK_KP_1, "keypad_end");
    registerValidKey(SDLK_KP_2, "keypad_down");
    registerValidKey(SDLK_KP_3, "keypad_pgdn");
    registerValidKey(SDLK_KP_0, "keypad_ins");
    registerValidKey(SDLK_KP_PERIOD, "keypad_del");
    registerValidKey(SDLK_F11, "f11");
    registerValidKey(SDLK_F12, "f12");
    registerValidKey(SDLK_F13, "f13");
    registerValidKey(SDLK_F14, "f14");
    registerValidKey(SDLK_F15, "f15");

    registerValidKey(SDLK_KP_ENTER, "keypad_enter");
    registerValidKey(SDLK_RALT, "alt");
    registerValidKey(SDLK_HOME, "home");
    registerValidKey(SDLK_UP, "up");
    registerValidKey(SDLK_PAGEUP, "pgup");
    registerValidKey(SDLK_LEFT, "left");
    registerValidKey(SDLK_RIGHT, "right");
    registerValidKey(SDLK_END, "end");
    registerValidKey(SDLK_DOWN, "down");
    registerValidKey(SDLK_PAGEDOWN, "pgdn");
    registerValidKey(SDLK_INSERT, "ins");
    registerValidKey(SDLK_DELETE, "del");
    registerValidKey(SDLK_BACKQUOTE, "`");
    registerValidKey(SDLK_BACKQUOTE | SHIFT_MOD, "~");
    registerValidKey(SDLK_PRINTSCREEN, "print_screen");
    registerValidKey(JOY_AXISR, "joy_axisr");
    registerValidKey(JOY_AXISX, "joy_axisx");
    registerValidKey(JOY_AXISY, "joy_axisy");
    registerValidKey(JOY_HAT_UP, "joy_hatup");
    registerValidKey(JOY_HAT_DOWN, "joy_hatdn");
    registerValidKey(JOY_HAT_RIGHT, "joy_hatrt");
    registerValidKey(JOY_HAT_LEFT, "joy_hatlt");
    registerValidKey(JOY_1, "joy1");
    registerValidKey(JOY_2, "joy2");
    registerValidKey(JOY_3, "joy3");
    registerValidKey(JOY_4, "joy4");
    registerValidKey(JOY_5, "joy5");
    registerValidKey(JOY_6, "joy6");
    registerValidKey(JOY_7, "joy7");
    registerValidKey(JOY_8, "joy8");
    registerValidKey(JOY_9, "joy9");
    registerValidKey(JOY_10, "joy10");
    registerValidKey(MOUSE1, "mouse1");
    registerValidKey(MOUSE2, "mouse2");
    registerValidKey(MOUSE3, "mouse3");
    registerValidKey(MOUSE4, "mouse4");
    registerValidKey(MOUSE5, "mouse5");
    registerValidKey(MOUSE6, "mouse6");
    registerValidKey(MOUSE7, "mouse7");
    registerValidKey(MOUSE8, "mouse8");
    registerValidKey(MOUSE_AXISX, "mouse_axisx");
    registerValidKey(MOUSE_AXISY, "mouse_axisy");
    registerValidKey(MOUSE_WHEEL, "mouse_wheel");
}

//------------------------------------------------------
unsigned int InputService::mapToKeyCode(const std::string &key) const {
    unsigned int code;
    ReverseKeyMap::const_iterator it = mReverseKeyMap.find(key);

    if (it != mReverseKeyMap.end())
        code = it->second;
    else
        // If we get here, then there is a key name that DarkEngine creates that
        // we didn't cover
        code = SDLK_UNKNOWN;

    return code;
}

//------------------------------------------------------
void InputService::addBinding(const std::string &keys,
                              const std::string &command,
                              InputEventMapper *mapper) {
    if (!mapper)
        mapper = mCurrentMapper;

    if (!mapper)
        mapper = mDefaultMapper;

    assert(mapper);

    std::string key = "";
    unsigned int modifier = 0;

    StringTokenizer bindtok(keys, '+');

    while (!bindtok.end()) {
        std::string token = bindtok.next();

        if (token == "shift")
            modifier |= SHIFT_MOD;
        else if (token == "ctrl")
            modifier |= CTRL_MOD;
        else if (token == "alt")
            modifier |= ALT_MOD;
        else {
            if (key == "")
                key = token;
            else {
                LOG_ERROR(
                    "InputService::addBinding: Invalid key combination %s",
                    keys.c_str());
                return;
            }
        }
    }

    const unsigned int code = mapToKeyCode(key);

    mapper->bind(code | modifier, command);

    LOG_DEBUG(
        "InputService: (Context %s) bound command '%s' to keyCode %d -'%s'",
        mapper->getName().c_str(), command.c_str(), code, key.c_str());
}

//------------------------------------------------------
Variant InputService::processCommand(const std::string &commandStr) {
    std::string cstr;
    cstr = stripComment(commandStr);

    std::transform(cstr.begin(), cstr.end(), cstr.begin(),
                   ::tolower); // Lowercase

    WhitespaceStringTokenizer tok(cstr, false);

    std::string command;
    if (!tok.pull(command)) {
        LOG_ERROR(
            "InputService::processCommand: Empty command string encountered!");
        return false;
    }

    // first token can be context if the next is bind
    InputEventMapper *mpr = findMapperForContext(command);

    if (mpr) {
        // is the next token bind?
        if (!tok.pull(command)) {
            LOG_ERROR("InputService::processCommand: Context without command!");
            return false;
        }
    }

    if (command == "bind") {
        if (mpr == NULL)
            mpr = mCurrentMapper;

        std::string keycode;

        if (!tok.pull(keycode)) {
            LOG_ERROR(
                "InputService::processCommand: bind command without keycode!");
            return false;
        }

        if (!tok.pull(command)) {
            LOG_ERROR(
                "InputService::processCommand: bind command without command!");
            return false;
        }

        addBinding(keycode, command, mpr);
        return true;
    } else if (command == "echo") {
        return tok.next();
    } else if (command == "set") {
        // must have two more tokens
        std::string var, val;

        if (!tok.pull(var))
            return false;

        if (!tok.pull(val))
            return false;

        setVariable(var, val);
        return true;
    } else if (command == "help") {
        // list all commands registered
        logCommands();
        return true;
    } else if (command == "show") {
        // list all commands registered
        logVariables();
        return true;
    } else if (command == "loadbnd") {
        // load bnd file
        loadBNDFile(tok.rest());
        return true;
    }

    ListenerMap::iterator it = mCommandTraps.find(command);
    if (it != mCommandTraps.end()) {
        InputEventMsg msg;
        msg.command = command;
        msg.params = tok.next();
        msg.event = IET_COMMAND_CALL;

        return callCommandTrap(msg);
    }

    std::string var, val;

    if (!tok.pull(var))
        return false;

    if (!tok.pull(val))
        return false;

    setVariable(var, val);
    return true;
}

//------------------------------------------------------
bool InputService::loadBNDFile(const std::string &fname) {
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, fname) == false)
        return false;

    Ogre::DataStreamPtr Stream =
        Ogre::ResourceGroupManager::getSingleton().openResource(
            fname, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

    while (!Stream->eof()) {
        std::string Line = Stream->getLine();
        processCommand(Line);
    }

    return true;
}

//------------------------------------------------------
void InputService::createBindContext(const std::string &ctx) {
    ContextToMapper::const_iterator it = mMappers.find(ctx);

    if (it == mMappers.end()) {
        InputEventMapper *mapper = new InputEventMapper(this, ctx);
        mMappers.insert(make_pair(ctx, mapper));
    } else {
        LOG_ERROR("InputService::createBindContext: Context already exists: %s",
                  ctx.c_str());
    }
}

//------------------------------------------------------
bool InputService::init() {
    mConfigService = GET_SERVICE(ConfigService);
    mRenderService = GET_SERVICE(RenderService);

    mConfigService->setParamDescription(
        "nonexclusive",
        "Enables non-exclusive input (Mouse and keyboard won't be blocked)");

    mRenderWindow = mRenderService->getRenderWindow();

    // Non-exclusive input - for debugging purposes
    // TODO: Nonexclusive
    mNonExclusive = false;

    // Last step: Get the loop service and register as a listener
    mLoopService = GET_SERVICE(LoopService);
    mLoopService->addLoopClient(this);

    initKeyMap();

    // we want relative SDL mouse mode!
    SDL_SetRelativeMouseMode(SDL_TRUE);

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
    mConfigService.reset();
}

//------------------------------------------------------
void InputService::loopStep(float deltaTime) { pollEvents(deltaTime); }

//------------------------------------------------------
void InputService::processKeyRepeat(float deltaTime) {
    if (mCurrentKey == SDLK_CLEAR)
        return;

    mKeyPressTime += deltaTime;

    assert(mCurrentDelay > 0);
    assert(mRepeatDelay > 0);

    // see if we overlapped
    while (mKeyPressTime > mRepeatDelay) {
        mKeyPressTime -= mCurrentDelay;
        mCurrentDelay = mRepeatDelay;

        if (mInputMode == IM_MAPPED)
            processKeyEvent(mCurrentKey, mCurrentMods, IET_KEYBOARD_HOLD);
    }
}

//------------------------------------------------------
void InputService::setBindContext(const std::string &context) {
    InputEventMapper *iemp = findMapperForContext(context);

    if (iemp)
        mCurrentMapper = iemp;
    else {
        mCurrentMapper = mDefaultMapper;
        LOG_ERROR("InputService::setBindContext: invalid context specified: "
                  "'%s', setting default context",
                  context.c_str());
    }
}

//------------------------------------------------------
std::string InputService::fillVariables(const std::string &src) const {
    // This is quite bad implementation actually.
    // Should use a StringTokenizer with custom rule ($ and some non-alpha like
    // space etc split).

    WhitespaceStringTokenizer st(src);

    if (st.end())
        return src;

    string result;

    bool first = true;

    while (!st.end()) {
        if (!first)
            result += " ";

        first = false;

        string var = st.next();

        if (var == "")
            continue;

        if (var.substr(0, 1) == "$") {
            ValueMap::const_iterator it =
                mVariables.find(var.substr(1, var.length() - 1));

            if (it != mVariables.end())
                result += it->second.toString();
            else
                result += var;

        } else
            result += var;
    }

    return result;
}

//------------------------------------------------------
InputEventMapper *InputService::findMapperForContext(const std::string &ctx) {
    ContextToMapper::const_iterator it = mMappers.find(ctx);

    if (it != mMappers.end())
        return it->second;
    else
        return NULL;
}

//------------------------------------------------------
/// char classifier (is comment)
struct IsComment : public std::unary_function<char, bool> {
    bool operator()(char c) { return c == ';'; }
};

//------------------------------------------------------
std::string InputService::stripComment(const std::string &cmd) {
    IsComment isC;

    string::const_iterator it = find_if(cmd.begin(), cmd.end(), isC);

    return string(cmd.begin(), it);
}

//------------------------------------------------------
Variant InputService::getVariable(const std::string &var) {
    ValueMap::const_iterator it = mVariables.find(var);

    if (it != mVariables.end())
        return it->second;
    else
        return Variant();
}

//------------------------------------------------------
void InputService::setVariable(const std::string &var, const Variant &val) {
    LOG_DEBUG("InputService::setVariable: '%s' -> %s", var.c_str(),
              val.toString().c_str());
    ValueMap::iterator it = mVariables.find(var);

    if (it != mVariables.end())
        it->second = val;
    else
        mVariables.insert(make_pair(var, val));
}

//------------------------------------------------------
void InputService::processKeyEvent(SDL_Keycode keyCode, unsigned int modifiers,
                                   InputEventType t) {
    // Some safety checks
    if (mInputMode == IM_DIRECT)
        return;

    if (!mCurrentMapper) {
        LOG_ERROR("InputService::processKeyEvent: Mapped input, but no mapper "
                  "set for the current state!");
        return;
    }

    std::string command;

    unsigned int code = keyCode;

    // translate from the modifiers to our mods
    if (modifiers & KMOD_SHIFT)
        code |= SHIFT_MOD;
    if (modifiers & KMOD_ALT)
        code |= ALT_MOD;
    if (modifiers & KMOD_CTRL)
        code |= CTRL_MOD;

    if (!mCurrentMapper->unmapEvent(keyCode, command)) {
        LOG_DEBUG("InputService: Encountered an unmapped key event %d",
                  keyCode);
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
        LOG_DEBUG("InputService: Key event '%d' (mapped to the command %s) "
                  "seems to have no trap present",
                  keyCode, msg.command.c_str());
    }
}

//------------------------------------------------------
void InputService::processMouseEvent(unsigned int id, InputEventType event) {
    // Some safety checks
    if (mInputMode == IM_DIRECT)
        return;

    if (!mCurrentMapper) {
        LOG_ERROR("InputService::processKeyEvent: Mapped input, but no mapper "
                  "set for the current state!");
        return;
    }

    // dispatch the key event using the mapper
    unsigned int button = id;
    button |= DARK_MOUSE_EVENT;

    std::string command;

    if (!mCurrentMapper->unmapEvent(button, command))
        return;

    InputEventMsg msg;
    std::pair<string, string> split = splitCommand(command);

    msg.command = split.first;
    msg.params = split.second;
    msg.event = event;

    if (command[0] == '+') {
        if (event == IET_MOUSE_PRESS)
            msg.params = 1.0f;
        else
            msg.params = 0.0f;
        command = command.substr(1);
    } else if (command[0] == '-') {
        if (event == IET_MOUSE_PRESS)
            msg.params = 0.0f;
        else
            msg.params = 1.0f;
        command = command.substr(1);
    }

    callCommandTrap(msg);
}

//------------------------------------------------------
void InputService::registerCommandTrap(const std::string &command,
                                       const ListenerPtr &listener) {
    if (mCommandTraps.find(command) != mCommandTraps.end()) {
        // Already registered command. LOG an error
        LOG_ERROR("InputService: The command %s already has a registered trap. "
                  "Not registering",
                  command.c_str());
        return;
    } else {
        LOG_INFO("InputService: The command '%s' is now registered",
                 command.c_str());
        mCommandTraps.insert(make_pair(command, listener));
    }
}

//------------------------------------------------------
void InputService::unregisterCommandTrap(const ListenerPtr &listener) {
    // Iterate through the trappers, find the ones with this listener ptr,
    // remove
    ListenerMap::iterator it = mCommandTraps.begin();

    while (it != mCommandTraps.end()) {
        ListenerMap::iterator pos = it++;

        if (pos->second == listener)
            mCommandTraps.erase(pos);
    }
}

//------------------------------------------------------
void InputService::unregisterCommandTrap(const std::string &command) {
    // Iterate through the trappers, find the ones with the given command name,
    // remove
    ListenerMap::iterator it = mCommandTraps.begin();

    while (it != mCommandTraps.end()) {
        ListenerMap::iterator pos = it++;

        if (pos->first == command)
            mCommandTraps.erase(pos);
    }
}

//------------------------------------------------------
void InputService::registerCommandAlias(const std::string &alias,
                                        const std::string &command) {
    mCommandAliases[alias] = command;
}

//------------------------------------------------------
void InputService::setDirectListener(DirectInputListener *listener) {
    if (mDirectListener)
        OPDE_EXCEPT("InputService: Direct input listener already registered",
        "InputService::setDirectListener");

    mDirectListener = listener;
};


//------------------------------------------------------
void InputService::pollEvents(float deltaTime) {
    // Process SDL queue
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            keyPressed(event.key);
            break;
        case SDL_KEYUP:
            keyReleased(event.key);
            break;
        case SDL_MOUSEMOTION:
            mouseMoved(event.motion);
            break;
        case SDL_MOUSEBUTTONUP:
            mousePressed(event.button);
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouseReleased(event.button);
            break;
        case SDL_QUIT:
            mLoopService->requestTermination();
            break;
        default:
            break;
        }
    }

    // Process the key repeat (but only for exclusive input)
    if (!mNonExclusive)
        processKeyRepeat(deltaTime / 1000.0f); // loop time is in
    // milis
}

//------------------------------------------------------
bool InputService::dealiasCommand(const std::string &alias,
                                  std::string &command) {
    AliasMap::const_iterator it = mCommandAliases.find(alias);

    if (it != mCommandAliases.end()) {
        command = it->second;
        return true;
    }

    return false;
}

//------------------------------------------------------
bool InputService::callCommandTrap(InputEventMsg &msg) {
    std::string cmd;

    // see if we found a new candidate
    if (dealiasCommand(msg.command, cmd)) {
        std::pair<string, string> split = splitCommand(cmd);
        msg.command = split.first;

        Variant param(split.second);
        msg.params = msg.params.as<float>() * param.as<float>();
    }

    ListenerMap::const_iterator it = mCommandTraps.find(msg.command);

    LOG_VERBOSE("InputService: Command trap '%s'", msg.command.c_str());

    if (it != mCommandTraps.end()) {
        (*it->second)(msg);
        return true;
    }

    return false;
}

//------------------------------------------------------
bool InputService::hasCommandTrap(const std::string &cmd) {
    if (mCommandTraps.find(cmd) != mCommandTraps.end())
        return true;

    return false;
}

//------------------------------------------------------
std::pair<std::string, std::string>
InputService::splitCommand(const std::string &cmd) const {
    WhitespaceStringTokenizer stok(cmd);

    std::pair<std::string, std::string> res = make_pair("", "");

    if (!stok.end())
        res.first = stok.next();

    if (!stok.end())
        res.second = stok.rest();

    return res;
}

//------------------------------------------------------
void InputService::logCommands() {
    ListenerMap::iterator it = mCommandTraps.begin();
    for (; it != mCommandTraps.end(); ++it) {
        LOG_INFO("InputService: Command %s", it->first.c_str());
    }
}

//------------------------------------------------------
void InputService::logVariables() {
    ValueMap::iterator it = mVariables.begin();
    for (; it != mVariables.end(); ++it) {
        LOG_INFO("InputService: Variable '%s' value '%s'", it->first.c_str(),
                 it->second.toString().c_str());
    }
}

//------------------------------------------------------
bool InputService::keyPressed(const SDL_KeyboardEvent &e) {
    // does the key have a character representation?
    if (!(e.keysym.sym & (1 << 30))) {
        mCurrentKey = e.keysym.sym;
        mCurrentMods = e.keysym.mod;
    }

    mKeyPressTime = 0;
    mCurrentDelay = mInitialDelay;

    if (mInputMode == IM_DIRECT) {
        if (mDirectListener) // direct event, dispatch to the current listener
            return mDirectListener->keyPressed(e);
    } else {
        processKeyEvent(e.keysym.sym, e.keysym.mod, IET_KEYBOARD_PRESS);
    }

    return false;
}

//------------------------------------------------------
bool InputService::keyReleased(const SDL_KeyboardEvent &e) {
    if (e.keysym.sym == mCurrentKey) {
        mCurrentKey = SDLK_CLEAR;
        mCurrentMods = 0;
    }

    if (mInputMode == IM_DIRECT) {
        if (mDirectListener) // direct event, dispatch to the current listener
            return mDirectListener->keyReleased(e);
    } else {
        processKeyEvent(e.keysym.sym, e.keysym.mod, IET_KEYBOARD_RELEASE);
    }

    return false;
}

//------------------------------------------------------
bool InputService::mouseMoved(const SDL_MouseMotionEvent &e) {

    if (mInputMode == IM_DIRECT) {
        if (mDirectListener) // direct event, dispatch to the current listener
            return mDirectListener->mouseMoved(e);
    } else {
        // TODO: dispatch the mouse movement using the mapper (axisx or axisy)
    }

    return false;
}

//------------------------------------------------------
bool InputService::mousePressed(const SDL_MouseButtonEvent &e) {
    if (mInputMode == IM_DIRECT) {
        if (mDirectListener) // direct event, dispatch to the current listener
            return mDirectListener->mousePressed(e);
    } else {
        processMouseEvent(e.button, IET_MOUSE_PRESS);
    }

    return false;
}

//------------------------------------------------------
bool InputService::mouseReleased(const SDL_MouseButtonEvent &e) {
    if (mInputMode == IM_DIRECT) {
        if (mDirectListener) // direct event, dispatch to the current listener
            return mDirectListener->mouseReleased(e);
    } else {
        processMouseEvent(e.button, IET_MOUSE_RELEASE);
    }

    return false;
}

//-------------------------- Factory implementation
std::string InputServiceFactory::mName = "InputService";

InputServiceFactory::InputServiceFactory() : ServiceFactory() {}

const std::string &InputServiceFactory::getName() { return mName; }

const uint InputServiceFactory::getMask() { return SERVICE_ENGINE; }

const size_t InputServiceFactory::getSID() { return InputService::SID; }

Service *InputServiceFactory::createInstance(ServiceManager *manager) {
    return new InputService(manager, mName);
}
} // namespace Opde
