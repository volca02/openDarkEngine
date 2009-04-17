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

#ifndef __LOGGER_H
#define __LOGGER_H

#include "config.h"

#include "OpdeSingleton.h"
#include <string>
#include <set>

namespace Opde {
	// Forward declaration
	class LogListener;

	/** Main logger class. This class is intended for logging purposes. Logging listeners, registered using registerLogListener method recieve logging messages formated by vsnprintf function */
	class OPDELIB_EXPORT Logger : public Singleton<Logger> {
		public:
			/// Logging level
			typedef enum {LOG_LEVEL_FATAL=0, LOG_LEVEL_ERROR, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_VERBOSE} LogLevel;

			typedef std::set<LogListener*> LogListenerSet;

		private:
			/** A set of listener classes */
			LogListenerSet mListeners;

			/** A global log level. Set by setLogLevel. All messages having higher log level are ignored */
			LogLevel mLoggingLevel;

			/** Message dispatching method. Sends the logging message to all log listeners */
			void dispatchLogMessage(LogLevel level, const std::string& msg);

		public:
			/** Constructor */
			Logger();

			/** Destructor. Does not deallocate the listeners, as this is not a wanted behavior. */
			~Logger();

			/** Log a message using printf syntax. */
			void log(LogLevel level, const char* fmt, ...);

			/** Register a new listener instance. The instance will receive all logging messages. */
			void registerLogListener(LogListener* listener);

			/** Unregister a listener instance */
			void unregisterLogListener(LogListener* listener);

            /** Translates the log level into something readable */
			const std::string getLogLevelStr(LogLevel level) const;

			/** C++ cout-like stream operator. Logs to INFO level */
			template<typename T> Logger& operator<< (const T& t);

			/** Logging level setter. */
			void setLogLevel(LogLevel level);

			/** Logging level setter - integer version */
			void setLogLevel(int level);

			// Singleton related stuff
			static Logger& getSingleton(void);
			static Logger* getSingletonPtr(void);
	};

	/** Log events listener class interface. The logEventRecieved gets called each time a message is logged. Do not use logging methods in implementation.
	* Also, the instance needs to be registered to the logging singleton Logger for the log listener to work.
	* Basicaly, this should implement a log writer to some target (File, Console, etc.).
	* @see Logger */
	class OPDELIB_EXPORT LogListener {
		public:
			LogListener();
			virtual ~LogListener();
			virtual void logMessage(Logger::LogLevel level, const std::string& msg) = 0;
	};



	// Printf-like logging helping defines
	#define LOG_FATAL(...) Logger::getSingleton().log(Logger::LOG_LEVEL_FATAL, __VA_ARGS__)
	#define LOG_ERROR(...) Logger::getSingleton().log(Logger::LOG_LEVEL_ERROR, __VA_ARGS__)
	#define LOG_INFO(...) Logger::getSingleton().log(Logger::LOG_LEVEL_INFO, __VA_ARGS__)

	// the debug+verbose loggers are conditionaly built in when the DEBUG flag is defined
	#ifdef OPDE_DEBUG
		#define LOG_DEBUG(...) Logger::getSingleton().log(Logger::LOG_LEVEL_DEBUG, __VA_ARGS__)
		#define LOG_VERBOSE(...) Logger::getSingleton().log(Logger::LOG_LEVEL_VERBOSE, __VA_ARGS__)
	#else
		#define LOG_DEBUG(...)
		#define LOG_VERBOSE(...)
	#endif

	// Shortcut to the Logger instance
	#define LOG Logger::getSingleton()
}

#endif
