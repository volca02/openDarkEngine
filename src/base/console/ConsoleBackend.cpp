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
		mCommandMap.clear();
		mCompletionMap.clear();
		mMessages.clear();
		mPosition = 0;
		mChanged = true;

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
		mMessages.push_back(text);	
		mChanged = true;
		
		// TODO: No position change if view is not at the end of the message list.
		mPosition += 1;
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

	void ConsoleBackend::write(const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug) {
		if (lml = LML_CRITICAL)
			addText("OgreLog: (" + name + " : " + message);
	}
	
	void ConsoleBackend::logMessage(LogLevel level, char *message) {
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

	bool ConsoleBackend::pullMessage(std::string& target) {
		if (mMessages.size() > 0) {
			list< string >::const_iterator first = mMessages.begin();

			target = (*first);
			mMessages.pop_front();
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
