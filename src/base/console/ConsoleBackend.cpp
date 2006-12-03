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
	
	ConsoleBackend::ConsoleBackend() {
		commandMap.clear();
		completionMap.clear();
		messages.clear();
		position = 0;
		changed = true;

		// Register as an ogre logger

		// TODO: This makes the console horribly slow for the first time it shows up. 
		// 3 Ways to solve: 
		//	* Pull texts as soon as possible (without console visible) and construct console gui as a first element in the application.
		//	* Log only our texts - do not be a ogre logging listener at all (But I want it to be - standardization)
		//  * Limit the widget pull only to max ~10 lines per frame

		// Or a combination of those...
		// Notice that limiting to LML_CRITICAL did not solve the issue

		// LogManager::getSingleton().addListener(this); 
	}
		
	void ConsoleBackend::addText(std::string text) {
		messages.push_back(text);		
		changed = true;
		
		// TODO: No position change if view is not at the end of the message list.
		position += 1;
	}

	bool ConsoleBackend::registerCommandListener(std::string Command, ConsoleCommandListener *listener) {
		map<string, ConsoleCommandListener *>::iterator commandIt = commandMap.find(Command);

		if (commandIt != commandMap.end()) { // already registered
			addText("Error: Command " + Command + " is already registered");
			return false;
		} else {
			commandMap.insert(make_pair(Command, listener));
			return true;
		}

		return false;
	}
		
	void ConsoleBackend::executeCommand(std::string Command) {
		addText(">" + Command);

		// Split the command on the first space... make it a Command PARAMETERS
		size_t space_pos = Command.find(' ');
		
		// If there are no parameters at all... as a default
		string command_part = Command;
		string command_parameters = "";

		if (space_pos != string::npos) { 
			// First substring to command, second to params
			command_part = Command.substr(0,space_pos);
			command_parameters = Command.substr(space_pos+1, Command.length() - (space_pos + 1));
		}

		// Try if it is a in-built command
		if (command_part == "commands") {
			map<string, ConsoleCommandListener *>::iterator commands = commandMap.begin();

			for (;commands != commandMap.end(); commands++) {
				addText("  " + (commands->first));
			}
			return;
		}
			
		// Find the command listener, as we are not the handler
		map<string, ConsoleCommandListener *>::iterator commands = commandMap.begin();

		for (;commands != commandMap.end(); commands++) {
				if ((commands->first) == command_part) {
					(commands->second)->commandExecuted(command_part, command_parameters);
					return;
				}
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

	void ConsoleBackend::write(const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug) {
		if (lml = LML_CRITICAL)
			addText("OgreLog: (" + name + " : " + message);
	}
	
	void ConsoleBackend::logMessage(LogLevel level, char *message) {
		std::string smessage(message);
		addText("LOG[" + string(logLevels[level]) + "] : " + smessage);
	}

	bool ConsoleBackend::getChanged() {
		if (changed) {
			changed = false;

			return true;
		}

		return false;
	}

	bool ConsoleBackend::pullMessage(std::string& target) {
		if (messages.size() > 0) {
			list< string >::const_iterator first = messages.begin();

			target = (*first);
			messages.pop_front();
			return true;
		} else
			return false;
	}
	
	ConsoleBackend& ConsoleBackend::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );  
	}
	
	ConsoleBackend* ConsoleBackend::getSingletonPtr(void) {
		return ms_Singleton;
	}
}
