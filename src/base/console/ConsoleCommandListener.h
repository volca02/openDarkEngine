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
 
#ifndef __consolecommandlistener_h
#define __consolecommandlistener_h

#include <string>

namespace Opde {
	
/** Abstract class ConsoleCommandListener. Defines an interface for classes, which want to register as a command executors */
class ConsoleCommandListener {
	public:
		/** Please override with a method that will handle the command */
		virtual void commandExecuted(std::string command, std::string parameters) = 0;
		
		virtual ~ConsoleCommandListener(void) {};
		
		// Just a thought: Would be possible to put "command?" and execute, and if the consoleBackend would detect a '?' at the end of command, would call this method instead of the commandExecuted
		// And should result in a small explanation...
    		//	void helpWanted(std::string command) = 0; 
};


}

#endif
