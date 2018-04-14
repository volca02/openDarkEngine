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

#include "InputService.h"
#include "OpdeException.h"
#include "ServiceCommon.h"
#include "StringTokenizer.h"
#include "logger.h"

#include <OgreResourceGroupManager.h>
#include <cctype>

using namespace std;

namespace Opde {

/*---------------------------------------------------*/
/*-------------------- InputEventMapper -------------*/
/*---------------------------------------------------*/
InputEventMapper::InputEventMapper(InputService *is, const std::string name)
    : mInputService(is), mName(name) {
    assert(mInputService != NULL);
}

//------------------------------------------------------
InputEventMapper::~InputEventMapper() {}

//------------------------------------------------------
bool InputEventMapper::unmapEvent(unsigned int code,
                                  std::string &unmapped) const {
    CodeToCommandMap::const_iterator it = mCodeToCommand.find(code);

    if (it != mCodeToCommand.end()) {
        unmapped = it->second;
        return true;
    } else
        return false;
}

//------------------------------------------------------
void InputEventMapper::bind(unsigned int code, const std::string &command) {
    mCodeToCommand[code] = command;
}
} // namespace Opde
