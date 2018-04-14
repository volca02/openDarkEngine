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
 *
 *		$Id$
 *
 *****************************************************************************/

#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "config.h"

#include "OpdeService.h"
#include "OpdeServiceManager.h"
#include "PlatformService.h"

namespace Opde {

/** @brief A platform. OS specific piece of code used to handle configuration
 * and data paths.
 */
class OPDELIB_EXPORT Platform {
public:
    Platform(PlatformService *owner);
    virtual ~Platform();

    /// Getter for system-wide configuration directory
    virtual std::string getGlobalConfigPath() const = 0;

    /// Getter for user-specific configuration directory
    virtual std::string getUserConfigPath() const = 0;

    /// Returns the directory separator string for the platform
    virtual std::string getDirectorySeparator() const = 0;

protected:
    /// Owner service
    PlatformService *mOwner;
};

} // namespace Opde

#endif
