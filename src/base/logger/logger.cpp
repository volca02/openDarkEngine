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


#include "logger.h"
#include <iostream>
#include <cstdarg>
#include <sstream>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning( disable : 4996 )
#endif

namespace Opde {

	struct IsNewline : public std::unary_function<char, bool> {
				bool operator()(char c) {
					return (c == '\n' || c == '\r');
				}
			};
	
	static IsNewline sIsNewLine;

	/** Just a helping array for the log string construction method */
	const char* LOG_LEVEL_STRINGS[5] = {"FATAL","ERROR","INFO ","DEBUG", "ALL  "};

	template<> Logger* Singleton<Logger>::ms_Singleton = 0;

	Logger::Logger() {
		mListeners.clear();
		mLoggingLevel = LOG_LEVEL_VERBOSE;
	}

	Logger::~Logger() {

	}

	Logger& Logger::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );
	}

	Logger* Logger::getSingletonPtr(void) {
		return ms_Singleton;
	}

	void Logger::dispatchLogMessage(LogLevel level, const std::string& msg) {
		LogListenerSet::iterator it = mListeners.begin();

		for (; it != mListeners.end(); it++) {
			LogListener* listener = *it;

			listener->logMessage(level, msg);
		}
	}

	void Logger::log(LogLevel level, const char *fmt, ...) {
		// Ignore the message if the level is too high
		if (mLoggingLevel < level)
			return;

		va_list argptr;
		char result[2048];

		va_start(argptr, fmt);
		vsnprintf(result, 2047, fmt, argptr);
		va_end(argptr);

		// This is how the log listener could work:
		// printf("Log (%s) : %s\n", LOG_LEVEL_STRINGS[level], result);
		
		// Now that the string is complete, we'll split it on newlines
		// to avoid losing headers on some lines
		std::string msg(result);
		
		std::string::iterator it = msg.begin(); 
		
		while (it != msg.end()) {
			std::string::iterator next = std::find_if(it, msg.end(), sIsNewLine);
			
			if (it != next) // drop empty ones
				dispatchLogMessage(level, std::string(it, next));
			
			it = next;
			
			if (it == msg.end())
				break;

			if (sIsNewLine(*it))
				it++;
		};

		
	}


	void Logger::registerLogListener(LogListener* listener) {
		mListeners.insert(listener);
	}

	void Logger::unregisterLogListener(LogListener* listener) {
		mListeners.erase(listener);
	}

	const std::string Logger::getLogLevelStr(LogLevel level) const {
	    if (level < 0)
            return "<????";

        if (level > LOG_LEVEL_VERBOSE)
            return ">????";

        return LOG_LEVEL_STRINGS[level];
	}


	template<typename T> Logger& Logger::operator<< (const T& t) {
		// Convert to stdstream
		std::ostringstream str;

		str << t;

		log(LOG_LEVEL_INFO,"%s",str.str().c_str());
		return *this;
	}

	void Logger::setLogLevel(LogLevel level) {
		mLoggingLevel = level;
	}

	void Logger::setLogLevel(int level) {
		switch (level) {
			case 0: setLogLevel(LOG_LEVEL_FATAL); break;
			case 1: setLogLevel(LOG_LEVEL_ERROR); break;
			case 2: setLogLevel(LOG_LEVEL_INFO); break;
			case 3: setLogLevel(LOG_LEVEL_DEBUG); break;
			case 4: setLogLevel(LOG_LEVEL_VERBOSE); break;
			default:
				setLogLevel(LOG_LEVEL_DEBUG);
				log(LOG_LEVEL_DEBUG, "Invalid logging level specified (%d), setting debug", level);
		}
	}

	LogListener::LogListener() {
	}

	LogListener::~LogListener() {
	}
}
