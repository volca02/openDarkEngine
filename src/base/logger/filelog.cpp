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
 *	  $Id$
 *
 *****************************************************************************/

#include "filelog.h"
#include "OpdeException.h"

#include <stdio.h>
#include <string>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace Opde {

FileLog::FileLog(const std::string &fname)
    : ofile(fname.c_str(), std::ios::out) {
    if (!ofile.is_open())
        OPDE_EXCEPT("Could not create output log file");
};

FileLog::~FileLog() { ofile.close(); };

void FileLog::logMessage(Logger::LogLevel level, const std::string &msg) {
    // Put in the severity/log level too
    ofile << "LOG [" << Logger::getSingleton().getLogLevelStr(level)
          << "] : " << msg << std::endl;
    ofile.flush();
}
} // namespace Opde
