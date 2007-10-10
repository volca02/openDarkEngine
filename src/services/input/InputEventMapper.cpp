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

#include "ServiceCommon.h"
#include "InputService.h"
#include "OpdeException.h"
#include "logger.h"
#include "StringTokenizer.h"

#include <OgreResourceGroupManager.h>
#include <cctype>

using namespace std;

namespace Opde {

    /*---------------------------------------------------*/
    /*-------------------- InputEventMapper -------------*/
    /*---------------------------------------------------*/
    InputEventMapper::InputEventMapper(InputService* is) : mInputService(is) {
    	assert(mInputService != NULL);
    }

    //------------------------------------------------------
    InputEventMapper::~InputEventMapper() {
    }

    //------------------------------------------------------
    bool InputEventMapper::unmapEvent(const std::string& event, SplitCommand& unmapped) {
		EventToCommandMap::const_iterator it = mEventToCommand.find(event);

		if (it != mEventToCommand.end()) {
			unmapped = it->second;
			return true;
		} else
			return false;
    }

    //------------------------------------------------------
    std::vector<std::string> InputEventMapper::getCommandEvents(const std::string& command) {

    }

    //------------------------------------------------------
    bool InputEventMapper::command(const std::string& cmd) {
    	WhitespaceStringTokenizer stok(cmd, false);

    	if (stok.end())
			return false;

    	// if the bind keyword is not present, return false
		if (stok.next() != "bind")
			return false;

		// validate the keyboard command

		if (stok.end())
			return false;

		string ev = stok.next();

		if (stok.end())
			return false;

		// The whole command, with parameters and binding type
		string wcommand = stok.next();

		// Split the command
		WhitespaceStringTokenizer cst(wcommand);

		if (cst.end())
			return false;

		SplitCommand spc;

		// peek at the first character
		std::string cmd1 = cst.next();

		if (cmd1 == "")
			return false;

		spc.type = CET_ONDOWN;

		if (cmd.substr(1,1) == "+") {
			spc.command = cmd1.substr(1);
			spc.type = CET_PEDGE;
		} else if (cmd.substr(1,1) == "-") {
			spc.command = cmd1.substr(1);
			spc.type = CET_NEDGE;
		}

		spc.command = cmd;
		spc.params = cst.rest();

		// The bnd files are not case sensitive. Thus, I convert the ev to lower case before validating/inserting
		// (Argh. The lower case conversion is way complicated in c++, the reason being locales and stuff like that. I simply ignore this here, OK?)
		transform(ev.begin(), ev.end(), ev.begin(), ::tolower);

		if (!mInputService->validateEventString(ev)) {
			LOG_ERROR("Encountered invalid event code: %s" ,ev.c_str());
			return false;
		}

		// event to command lookup
		pair<EventToCommandMap::const_iterator, bool> res = mEventToCommand.insert(make_pair(ev, spc));

		LOG_DEBUG("InputEventMapper::command: Mapping '%s' to cmd '%s'", ev.c_str(), wcommand.c_str());

		if (res.second) { // did insert
			return true;
		}

		// Reverse lookup helper (command to keys)
		// TODO: Code. Not needed for now. I guess that this one should search on command part, not the whole command

		return false;
    }

}
