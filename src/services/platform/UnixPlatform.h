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

#ifndef __UNIXPLATFORM_H
#define __UNIXPLATFORM_H

#include "config.h"

#include "OpdeService.h"
#include "OpdeServiceManager.h"
#include "Platform.h"

namespace Opde {

/** @brief Unix platform. Unix/Linux (Posix OS) specific platform handling code.
 */
class UnixPlatform : public Platform {
public:
    UnixPlatform(PlatformService *owner);
    virtual ~UnixPlatform();

    virtual std::string getGlobalConfigPath() const;

    virtual std::string getUserConfigPath() const;

    virtual std::string getDirectorySeparator() const;

private:
    static const std::string msOpdeHomeDirName;
};

} // namespace Opde

#endif
