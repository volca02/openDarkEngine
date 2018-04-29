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

#include <SDL2/SDL.h>
#include <functional>

#include "ServiceCommon.h"

#include "Variant.h"
#include "InputEventMapper.h"
#include "OpdeService.h"
#include "OpdeServiceManager.h"
#include "SharedPtr.h"
#include "loop/LoopCommon.h"
#include "InputCommon.h"

#include <deque>

namespace Opde {

/** @brief Input service - service which handles user input, and user input
 * mapping
 * @todo Tab completion for the service (preferably including variables)
 * @todo Variable registration, descriptions, enumeration (So one can get list
 * of variables, get their value and description)
 * @todo Mouse axis handling
 * @todo Joystick handling
 * @todo +/without + binding modes - how they differ, how to implement the one
 * with key repeat...
 * @todo BND file writing ability (needs cooperation with nonexistent platform
 * service)
 */
class InputService : public ServiceImpl<InputService>, public LoopClient {
public:
    InputService(ServiceManager *manager, const std::string &name);
    virtual ~InputService();

    /// Creates bind context (e.g. a switchable context, that maps events to
    /// commands if IM_MAPPED is active, using the mapper of this context)
    void createBindContext(const std::string &ctx);

    /// Set a current bind context
    void setBindContext(const std::string &context);

    /// Replaces all occurences of $variable with it's value
    std::string fillVariables(const std::string &src) const;

    /// Sets the input mode to either direct or mapped (IM_DIRECT/IM_MAPPED)
    void setInputMode(InputMode mode) { mInputMode = mode; };

    /** Loads a bindings file, possibly rebinding the current bindings
     * @param filename The filename of the bnd file to load
     * @todo dcontext The default context for bind commands not preceeded with
     * the context information. Defaults to the current context if not specified
     */
    bool loadBNDFile(const std::string &filename);

    /// Adds a command to the pool
    Variant processCommand(const std::string &CommandString);

    /// TODO:	void saveBNDFile(const std::string& filename);

    /// Declares a variable. Returns a reference to it
    Variant &createVariable(const std::string &var, const Variant &d_val);

        /// Variable getter (for mouse senitivity, etc)
        Variant &getVariable(const std::string &var);
    const Variant &getVariable(const std::string &var) const;

    /// Variable setter (for mouse senitivity, etc)
    void setVariable(const std::string &var, const Variant &val);

    /// Definition of the command listener
    using Listener = std::function<void(const InputEventMsg&)>;

    /// Registers a command listener
    void registerCommandTrap(const std::string &command,
                             const Listener &listener);

    /// Unregisters a command listener by it's name
    void unregisterCommandTrap(const std::string &command);

    /** registers a command alias
     * @param alias The alias to use
     * @param command the command that gets executed when the alias is
     * encountered
     */
    void registerCommandAlias(const std::string &alias,
                              const std::string &command);

    /// Sets a direct listener (only notified when IM_DIRECT is active)
    void setDirectListener(DirectInputListener *listener);

    /// Clears the direct listener mapping
    void unsetDirectListener() { mDirectListener = NULL; };

    /// polls events from SDL. Called in loopStep
    void pollEvents(float deltaTime);

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
    unsigned int mapToKeyCode(const std::string &key) const;

    /// registers SDL keysym to textual representation and inverse mappings
    void registerValidKey(int kc, const std::string &txt);

    /** Calls a trap for a command
     * @warning Modifies the msg parameter upon dealiasing
     * @returns true if the command was found, false otherwise
     */
    bool callCommandTrap(InputEventMsg &msg);

    /// returns true if the command trap exists for a given command
    bool hasCommandTrap(const std::string &cmd);

    /// Splits the given command on first whitespace to command an parameters
    /// parts
    std::pair<std::string, std::string>
    splitCommand(const std::string &cmd) const;

    /// Logs all available commands
    void logCommands();

    /// Logs all available variables and their values
    void logVariables();

    /// Processes the received key event with current mapper, and if it finds a
    /// match, sends an event
    void processKeyEvent(SDL_Keycode keyCode, unsigned int modifiers,
                         InputEventType t);

    /// Processes joystick or mouse event (axis movement, etc)
    void processMouseEvent(unsigned int Id, InputEventType Event);

    // ---- input events ----
    bool keyPressed(const SDL_KeyboardEvent &e);
    bool keyReleased(const SDL_KeyboardEvent &e);

    bool mouseMoved(const SDL_MouseMotionEvent &e);
    bool mousePressed(const SDL_MouseButtonEvent &e);
    bool mouseReleased(const SDL_MouseButtonEvent &e);

    /// Finds a mapper (Or NULL) for the specified context
    InputEventMapper *findMapperForContext(const std::string &ctx);

    /// Strips a comment (any text after ';' including that character)
    std::string stripComment(const std::string &cmd);

    /// Defines a new input binding in specified mapper
    void addBinding(const std::string &keys, const std::string &command,
                    InputEventMapper *mapper);

    /** Gives a command found for the given alias, if any
     * @param alias The aliased command
     * @param command the string that gets filled with the dealiased version of
     * the command
     * @return true if the alias was found and command was dealiased
     */
    bool dealiasCommand(const std::string &alias, std::string &command);

    /// Named context to an event mapper map
    typedef std::map<std::string, std::unique_ptr<InputEventMapper>> ContextToMapper;

    /// string variable name to variant map
    typedef std::map<std::string, Variant> ValueMap;

    /// map of SDL scancode to the code text
    typedef std::map<unsigned int, std::string> KeyMap; // (lower case please)

    /// map of the command text to the (int) ois key code
    typedef std::map<std::string, int> ReverseKeyMap; // (lower case please)

    /// map of command text to the handling listener
    typedef std::map<std::string, Listener> ListenerMap;

    /// string variable name to dealiased value
    typedef std::map<std::string, std::string> AliasMap;

    /// map of the context name to the mapper
    ContextToMapper mMappers;

    /// Variable list
    ValueMap mVariables;

    /// Key map
    KeyMap mKeyMap;

    /// Reverse key map
    ReverseKeyMap mReverseKeyMap;

    /// Current mapper
    InputEventMapper *mCurrentMapper;
    InputEventMapper *mDefaultMapper;

    /// Current input mode
    InputMode mInputMode;

    /// Map of the command trappers
    ListenerMap mCommandTraps;

    /// Current direct listener
    DirectInputListener *mDirectListener;

    /// Key Repeat: Initial delay (between first and second key repeats)
    float mInitialDelay;
    /// Key Repeat: Repeat delay (between second and latter key repeats)
    float mRepeatDelay;
    /// Key Repeat: Current key time
    float mKeyPressTime;
    /// Key Repeat: Current target delay time
    float mCurrentDelay;
    /// Key Repeat: Current key pressed
    SDL_Keycode mCurrentKey;
    /// Current modifiers (ALT, CTRL...)
    unsigned int mCurrentMods;

    /// Renderer window we listen on
    Ogre::RenderWindow *mRenderWindow;

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
class InputServiceFactory : public ServiceFactory {
public:
    InputServiceFactory();
    ~InputServiceFactory(){};

    /** Creates a InputService instance */
    Service *createInstance(ServiceManager *manager);

    virtual const std::string &getName();

    virtual const uint getMask();

    virtual const size_t getSID();

private:
    static std::string mName;
};
} // namespace Opde

#endif
