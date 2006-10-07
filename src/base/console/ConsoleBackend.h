#ifndef __consolebackend_h
#define __consolebackend_h

#include <string>
#include <map>

#include "OgreLogManager.h"

#include "ConsoleCommandListener.h"

namespace Opde {
	/** Backend class, used for commands processing.
	* A singleton class, used to insert texts to console and to call Command Listeners 
	* \todo TODO: Use a uniform approach at the singletons, then rewrite this one to use it */
	class ConsoleBackend : public Ogre::LogListener {
		private:
			/** Map of the string to the Listeners which handle them */
			std::map<std::string, ConsoleCommandListener *> commandMap;
		
			/** Command accelerator - tab completion map*/
			std::map<std::string, std::set<std::string> > completionMap;
		
			/** Console texts list */
			std::list< std::string > messages;
		
			/** Current view position. Use method scroll to move the actual view */
			unsigned int position;

			/** Singleton's instance */ 
			static ConsoleBackend* pinstance;
		
			/** Internal method for adding text rows */
			void addText(std::string text);
				
			/** Indicates true if the console text / scroll changed till last time and should be redrawn */
			bool changed;

		protected:
			/** constructor. Do not use. Use the methods instead. */ 
			ConsoleBackend();

		public:
			/** Setups the consoleBackend */
			static void setup();

			/** Return a singleton handle to the Real backend */
			static ConsoleBackend* getInstance();	
		
			/** Will register the command Command with the ConsoleCommandListener listener
			 * \return true if sucessful */
			bool registerCommandListener(std::string Command, ConsoleCommandListener *listener);
		
			/** execute the command, given on the commandline. Will tokenize by " " to find the first word, try to find that word as a command, and if sucessfull, will execute the commandListener's method commandExecuted */
			void executeCommand(std::string Command);
		
			/** Will puts a tab completion message, or just complete the command if only one candidate is found */
			std::string tabComplete(std::string Text);
		
			/* Writes a simple message to the console */
			void putMessage(std::string text);
		
			/** Ogre's log listener implementation. Used as a to console logger for the ogre Logging system. This means that one can se the ogre logger to write messages to console too */
			virtual void write(const Ogre::String &name, const Ogre::String &message, Ogre::LogMessageLevel lml=Ogre::LML_NORMAL, bool maskDebug=false);
			
			/** Returns true, if the console text was changed from last time, and resets the indicator - asking twice will return true,false */
			bool getChanged();

			/** Pulls a new message out of the queue of new messages, and deletes it */
			bool pullMessage(std::string& target);
	};

}


#endif
