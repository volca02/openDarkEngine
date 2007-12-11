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
 *****************************************************************************/

#include "ConsoleBackend.h"

namespace Opde {
	using namespace Ogre;
	using namespace std;

	const char* logLevels[5] = {"FATAL","ERROR","INFO","DEBUG","VERBOSE"};

	template<> ConsoleBackend* Singleton<ConsoleBackend>::ms_Singleton = 0;

	ConsoleBackend::ConsoleBackend(unsigned int text_history) : mTextHistory(text_history), mCommandMap(), mCompletionMap() {
		mCommandMap.clear();
		mCompletionMap.clear();
		mMessages.clear();
		mChanged = true;

		// Register as an ogre logger
		//LogManager::getSingleton().getDefaultLog()->addListener(this);
	}

	void ConsoleBackend::addText(std::string text) {
		// split the text on newlines, to aid line counting
		std::vector< String > lines = StringUtil::split(text, "\r\n");

		std::vector< String >::iterator it = lines.begin();



		for (;it != lines.end(); it++) {
			mMessages.push_back(*it);
		}

		// if the size is greater the mTextHistory, remove till sufficient
		while (mMessages.size() > mTextHistory)
			mMessages.pop_front();

		mChanged = true;
	}

	void ConsoleBackend::registerCommandListener(std::string command, ConsoleCommandListener *listener) {
		map<string, ConsoleCommandListener *>::iterator commandIt = mCommandMap.find(command);

		if (commandIt != mCommandMap.end()) { // already registered
			LOG_DEBUG("ConsoleBackend::registerCommandListener: Command %s is already registered, reregistering the listener pointer",  command.c_str());
			commandIt->second = listener;
		} else {
			mCommandMap.insert(make_pair(command, listener));
		}
	}

	void ConsoleBackend::setCommandHint(std::string command, std::string hint) {
		mHintMap.insert(make_pair(command, hint));
	}

	void ConsoleBackend::executeCommand(std::string command) {
		addText(">" + command);

		// Split the command on the first space... make it a Command PARAMETERS
		size_t space_pos = command.find(' ');

		// If there are no parameters at all... as a default
		string command_part = command;
		string command_parameters = "";

		if (space_pos != string::npos) {
			// First substring to command, second to params
			command_part = command.substr(0,space_pos);
			command_parameters = command.substr(space_pos+1, command.length() - (space_pos + 1));
		}

		// Try if it is a in-built command
		if (command_part == "commands" || command_part == "?") {
			map<string, ConsoleCommandListener *>::iterator commands = mCommandMap.begin();

			for (;commands != mCommandMap.end(); commands++) {

				// Try to look for a hint text
				map<string, string>::const_iterator hintIt = mHintMap.find(commands->first);

				if (hintIt != mHintMap.end()) // if a hint text was found, use it
					addText("  " + (commands->first) + " - " + hintIt->second);
				else {
					addText("  " + (commands->first));
				}
			}
			return;
		}

		// Find the command listener, as we are not the handler
		map<string, ConsoleCommandListener *>::iterator commandIt = mCommandMap.find(command_part);

		if (commandIt != mCommandMap.end()) {
			(commandIt->second)->commandExecuted(command_part, command_parameters);
			return;
		}

		// command not found...
		addText("Error: Command " + command_part + " not understood!");
	}

	std::string ConsoleBackend::tabComplete(std::string Text) {
		//TODO: Code
		return Text;
	}

	void ConsoleBackend::putMessage(std::string text) {
		addText(text);
	}

	void ConsoleBackend::messageLogged( const String& message, LogMessageLevel lml, bool maskDebug, const String &logName ) {
		if (lml = LML_CRITICAL)
			addText("OgreLog: (" + logName + ") : " + message);
	}

	void ConsoleBackend::logMessage(Logger::LogLevel level, char *message) {
		std::string smessage(message);
		addText("LOG[" + string(logLevels[level]) + "] : " + smessage);
	}

	bool ConsoleBackend::getChanged() {
		if (mChanged) {
			mChanged = false;

			return true;
		}

		return false;
	}

	void ConsoleBackend::pullMessages(std::vector<Ogre::String>& target, int pos, unsigned int lines) {
		// add the lines from the mMessages backwards

		std::deque < Ogre::String >::iterator it;

		int size = mMessages.size();

		if (pos >= size) {
			pos = size - 1;
		}

		if (pos < 0) { // from end
			pos = size + (pos + 1);
			
			if (size - pos < lines) // if there could be more lines visible, justify
                pos = size - lines;

            if (pos < 0) { // if the resulting position is less than zero, justify both position and size
                pos = 0;
                
                if (lines > size)
					lines = size;
            }

            it = mMessages.begin() + pos;
		} else {
            // If the pos was negative, fill the screen
            it = mMessages.begin() + pos;
            lines = (size - pos) > lines ? lines : (size - pos);
		}

		for (;lines > 0; --lines, ++it) {
			target.push_back(*it);
		}
	}

	ConsoleBackend& ConsoleBackend::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );
	}

	ConsoleBackend* ConsoleBackend::getSingletonPtr(void) {
		return ms_Singleton;
	}

}
