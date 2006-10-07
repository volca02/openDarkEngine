#ifndef __consolecommandlistener_h
#define __consolecommandlistener_h

#include <string>

/** Abstract class ConsoleCommandListener. Defines an interface for classes, which want to register as a command executors */
class ConsoleCommandListener {
	public:
		/** Please override with a method that will handle the command */
		virtual void commandExecuted(std::string command, std::string parameters) = 0;
		
		// Just a thought: Would be possible to put command? and execute, and if the consoleBackend would detect a '?' at the end of command, would call this method instead of the commandExecuted
		// And should result in a small explanation...
    		//	void helpWanted(std::string command) = 0; 
};



#endif
