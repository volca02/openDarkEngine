#include "ConsoleBackend.h"	

namespace Opde {
	using namespace Ogre;
	using namespace std;


	// Static member definition:
	ConsoleBackend* ConsoleBackend::pinstance;

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

	void ConsoleBackend::setup() {
		pinstance = NULL;
	}

	ConsoleBackend* ConsoleBackend::getInstance() {
		if (pinstance == NULL) 
			pinstance = new ConsoleBackend();

		return pinstance;
	}
		
	void ConsoleBackend::addText(string text) {
		messages.push_back(text);		
		changed = true;
		
		// TODO: No position change if view is not at the end of the message list.
		position += 1;
	}

	bool ConsoleBackend::registerCommandListener(string Command, ConsoleCommandListener *listener) {
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
		
	void ConsoleBackend::executeCommand(string Command) {
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

	std::string ConsoleBackend::tabComplete(string Text) {
		//TODO: Code
		return Text;
	}
		
	void ConsoleBackend::putMessage(std::string text) {
		addText(text);
	}

	void ConsoleBackend::write(const String &name, const String &message, LogMessageLevel lml, bool maskDebug) {
		if (lml = LML_CRITICAL)
			addText("OgreLog: (" + name + " : " + message);
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
}
