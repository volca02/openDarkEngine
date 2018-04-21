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
 *****************************************************************************/

#ifndef __INPUTCOMMON_H
#define __INPUTCOMMON_H

#include "Variant.h"

// fwds so we don't have to pull in SDL2/SDL.h here
struct SDL_KeyboardEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseButtonEvent;

namespace Opde {

/// Input event types
enum InputEventType {
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
};

// SDL uses bit 30 for character-less keys...
#define SHIFT_MOD (1 << 29)
#define CTRL_MOD (1 << 28)
#define ALT_MOD (1 << 27)
#define DARK_JOY_EVENT (1 << 26)
#define DARK_MOUSE_EVENT (1 << 25)

enum DarkJoyStickEvents {
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

enum DarkMouseEvents {
    MOUSE1 = DARK_MOUSE_EVENT + 0,
    MOUSE2 = DARK_MOUSE_EVENT + 1,
    MOUSE3 = DARK_MOUSE_EVENT + 2,
    MOUSE4 = DARK_MOUSE_EVENT + 3,
    MOUSE5 = DARK_MOUSE_EVENT + 4,
    MOUSE6 = DARK_MOUSE_EVENT + 5,
    MOUSE7 = DARK_MOUSE_EVENT + 6,
    MOUSE8 = DARK_MOUSE_EVENT + 7,
    MOUSE_AXISX,
    MOUSE_AXISY,
    MOUSE_WHEEL
};

/// Input event
struct InputEventMsg {
    InputEventType event;
    // unmapped command, or empty
    std::string command;
    // Parameters of the command
    Variant params;
} ;

/// The state of input modifiers
enum InputModifierState {
    /// Shift key modifier
    IST_SHIFT = 1,
    /// Alt key modifier
    IST_ALT = 2,
    /// Control key modifier
    IST_CTRL = 4
};

/// The input mode - direct or translated to the commands
enum InputMode {
    /// Direct mode
    IM_DIRECT = 1,
    /// Translated mode
    IM_MAPPED
};

/// Listener for the direct - unfiltered events. Typically one per application
/// (GUIService for example)
class DirectInputListener {
public:
    virtual ~DirectInputListener(){};

    virtual bool keyPressed(const SDL_KeyboardEvent &e) = 0;
    virtual bool keyReleased(const SDL_KeyboardEvent &e) = 0;

    virtual bool mouseMoved(const SDL_MouseMotionEvent &e) = 0;
    virtual bool mousePressed(const SDL_MouseButtonEvent &e) = 0;
    virtual bool mouseReleased(const SDL_MouseButtonEvent &e) = 0;
};

} // namespace Opde

#endif /* __INPUTCOMMON_H */
