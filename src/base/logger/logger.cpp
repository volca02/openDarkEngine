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


#include "logger.h"
#include <iostream>
#include <cstdarg>
#include <sstream>

namespace Opde {

	/** Just a helping array for the log string construction method */
	const char* LOG_LEVEL_STRINGS[5] = {"FATAL","ERROR","INFO ","DEBUG", "ALL  "};

	template<> Logger* Singleton<Logger>::ms_Singleton = 0;

	Logger::Logger() {
		listeners.clear();
		loggingLevel = LOG_DEBUG;
	}

	Logger::~Logger() {

	}

	Logger& Logger::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );
	}

	Logger* Logger::getSingletonPtr(void) {
		return ms_Singleton;
	}

	void Logger::dispatchLogMessage(LogLevel level, char *message) {
		std::set<LogListener*>::iterator it = listeners.begin();

		for (; it != listeners.end(); it++) {
			LogListener* listener = *it;

			listener->logMessage(level, message);
		}
	}

	void Logger::log(LogLevel level, char *fmt, ...) {
		// Ignore the message if the level is too high
		if (loggingLevel < level)
			return;

		va_list argptr;
		char result[2048];

		va_start(argptr, fmt);
		vsnprintf(result, 2047, fmt, argptr);
		va_end(argptr);

		// This is how the log listener could work:
		// printf("Log (%s) : %s\n", LOG_LEVEL_STRINGS[level], result);

		dispatchLogMessage(level, result);
	}


	void Logger::registerLogListener(LogListener* listener) {
		listeners.insert(listener);
	}

	const std::string Logger::getLogLevelStr(LogLevel level) const {
	    if (level < 0)
            return "<????";

        if (level > LOG_VERBOSE)
            return ">????";

        return LOG_LEVEL_STRINGS[level];
	}


	template<typename T> Logger& Logger::operator<< (const T& t) {
		// Convert to stdstream
		std::ostringstream str;

		str << t;

		log(LOG_INFO,"%s",str.str().c_str());
		return *this;
	}

	void Logger::setLogLevel(LogLevel level) {
		loggingLevel = level;
	}

}
