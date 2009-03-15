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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __INPUTCONTEXTMAPPER_H
#define __INPUTCONTEXTMAPPER_H

#include "SharedPtr.h"

#include <string>
#include <map>

namespace Opde {
	class InputService;

	/** @brief Input Event Mapper - Manages bindings in a specific context
	* This class remaps input events to text commands according to the bindings.
	*/
	class InputEventMapper {
		public:
			/// Command event transformation
			enum CommandEventType {
				/// Go positive on keypress, negative on release (Edge detection)
				CET_PEDGE,
				/// Go negative on keypress, positive on release (Edge detection)
				CET_NEDGE,
				/// call command every time the state is DOWN (pressed)
				CET_NORMAL
			};

			/// Parsed command (bind format: "[+/-]command (params)")
			typedef struct {
				/// Type of the event to handle
				CommandEventType type;
				/// Command to emit on the event
				std::string command;
			} Command;

			InputEventMapper(InputService* is);
			virtual ~InputEventMapper();

			/** Unmaps an event to a command.
			* @param event The event to unmap
			* @param unmapped the command that this event is mapped to
			* @return true if a command was found for the event, false otherwise */
			bool unmapEvent(const std::string& event, Command& unmapped);

			/** Gets the list of events leading to a command */
			std::vector<std::string> getCommandEvents(const std::string& command);

		protected:

			typedef std::map<std::string, Command> EventToCommandMap;

			/// Map of command
			EventToCommandMap mEventToCommand;

			/// The input service owner of this event mapper
			InputService* mInputService;
	};

	/// Shared pointer to game service
	typedef shared_ptr<InputEventMapper> InputEventMapperPtr;

}


#endif
