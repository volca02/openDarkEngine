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
 *
 *		$Id$
 *
 *****************************************************************************/

#ifndef __OGREOPDELOGCONNECTOR_H
#define __OGREOPDELOGCONNECTOR_H

#include "config.h"

#include "logger.h"

#include <OgreLog.h>

namespace Opde {

/** Ogre<->Opde log connector. Logs messages from ogre as a special kind of opde
 * log messages */
class OPDELIB_EXPORT OgreOpdeLogConnector : public Ogre::LogListener {
protected:
    Logger *mOpdeLogger;

public:
    OgreOpdeLogConnector(Logger *opdeLogger);
    virtual ~OgreOpdeLogConnector();

    virtual void messageLogged(const Ogre::String &message,
                               Ogre::LogMessageLevel lml, bool maskDebug,
                               const Ogre::String &logName,
                               bool &skipThisMessage);
};
} // namespace Opde

#endif
