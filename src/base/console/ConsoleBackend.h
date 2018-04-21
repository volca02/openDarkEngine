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

#ifndef __consolebackend_h
#define __consolebackend_h

#include "config.h"

#include <map>
#include <string>

#include "OgreLogManager.h"

#include "ConsoleCommandListener.h"

#include "OpdeSingleton.h"
#include "logger/logger.h"

namespace Opde {

/** Backend class, used for commands processing, and console text memmory
 * A singleton class, used to insert texts to console and to call Command
 * Listeners */
class ConsoleBackend : public Singleton<ConsoleBackend>,
                       public Ogre::LogListener,
                       public Opde::LogListener {
public:
    /** A pair - level, message */
    typedef std::pair<size_t, Ogre::String> Message;

private:
    /** Map of the string to the Listeners which handle them */
    std::map<std::string, ConsoleCommandListener *> mCommandMap;

    /** Map of the string to the Listeners which handle them */
    std::map<std::string, std::string> mHintMap;

    /** Command accelerator - tab completion map*/
    std::map<std::string, std::set<std::string>> mCompletionMap;

    /** Console texts list */
    std::deque<Message> mMessages;

    /** Internal method for adding text rows */
    void addText(std::string text, size_t level = 2);

    /** Indicates true if the console text / scroll changed till last time and
     * should be redrawn */
    bool mChanged;

    unsigned int mTextHistory;

public:
    /** constructor */
    ConsoleBackend(unsigned int text_history = 1000);

    /** destructor */
    ~ConsoleBackend();

    /** Will register the command Command with the ConsoleCommandListener
     * listener
     * @note When the command is already registered, the listener will be
     * reregistered, allowing this to be called in the constructors */
    void registerCommandListener(std::string Command,
                                 ConsoleCommandListener *listener);

    /** Register a help (or a short description) for a command
     * @param command Command name
     * @param hint Hint text
     */
    void setCommandHint(std::string command, std::string hint);

    /** execute the command, given on the commandline. Will tokenize by " " to
     * find the first word, try to find that word as a command, and if
     * sucessfull, will execute the commandListener's method commandExecuted */
    void executeCommand(std::string Command);

    /** Will puts a tab completion message, or just complete the command if only
     * one candidate is found */
    std::string tabComplete(std::string Text);

    /* Writes a simple message to the console */
    void putMessage(std::string text, size_t level = 2);

    /** Ogre's log listener implementation. Used as a to console logger for the
     * ogre Logging system. This means that one can se the ogre logger to write
     * messages to console too */
    virtual void messageLogged(const Ogre::String &message,
                               Ogre::LogMessageLevel lml, bool maskDebug,
                               const Ogre::String &logName,
                               bool &skipThisMessage);

    /** Opde logging method implementation */
    virtual void logMessage(Logger::LogLevel level, const std::string &msg);

    /** Returns true, if the console text was changed from last time, and resets
     * the indicator - asking twice will return true,false */
    bool getChanged();

    /** informs the backend a change happened (manually). This is here to aid
     * the console frontend with rendering changes (page changes, scrolling) */
    void setChanged() { mChanged = true; };

    /** Returns the size of history. In lines. */
    unsigned int getHistorySize() { return mTextHistory; };

    /** Scroll the view window a defined number of lines */
    void scroll(int lines);

    /** Pulls a set of messages out of the memory of messages.
    @param linenum The start line to load. -1 means we will pull up to #lines
    from end of the vector
    @param lines the maximal count of lines to load
    */
    void pullMessages(std::vector<ConsoleBackend::Message> &target, int pos,
                      unsigned int lines);

    // Singleton stuff
    static ConsoleBackend &getSingleton(void);
    static ConsoleBackend *getSingletonPtr(void);
};

} // namespace Opde

#endif
