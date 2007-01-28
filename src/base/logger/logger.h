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
 
#ifndef __LOGGER_H
#define __LOGGER_H

#include "config.h"
#include "OpdeSingleton.h"
#include <set>

namespace Opde {
	/** Logging levels 
	* @todo << operator for this enum maybe? */
	typedef enum{LOG_FATAL=0, LOG_ERROR, LOG_INFO, LOG_DEBUG, LOG_VERBOSE} LogLevel;

	
	/** Log events listener class interface. The logEventRecieved gets called each time a message is logged. Do not use logging methods in implementation. 
	* Also, the instance needs to be registered to the logging singleton Logger for the log listener to work.
	* Basicaly, this should implement a log writer to some target (File, Console, etc.).
	* @see Logger */
	class LogListener {
		public:
			LogListener() {};
			virtual ~LogListener() {};
			virtual void logMessage(LogLevel level, char *message) = 0;
	};
	
	
	/** Main logger class. This class is intended for logging purposes. Logging listeners, registered using registerLogListener method recieve logging messages formated by vsnprintf function */
	class Logger : public Singleton<Logger> {
		private:
			/** A set of listener classes */
			std::set<LogListener*> listeners;
		
			/** A global log level. Set by setLogLevel. All messages having higher log level are ignored */
			LogLevel loggingLevel;
			
			/** Message dispatching method. Sends the logging message to all log listeners */
			void dispatchLogMessage(LogLevel level, char *msg);
		
		public:
			/** Constructor */
			Logger();
		
			/** Destructor. Does not deallocate the listeners, as this is not a wanted behavior. */
			~Logger();
			
			/** Log a message using printf syntax. */
			void log(LogLevel level, char* fmt, ...);
		
			/** Register a new listener instance. The instance will receive all logging messages. */
			void registerLogListener(LogListener* listener);
	
			/** C++ cout-like stream operator. Logs to INFO level */
			template<typename T> Logger& operator<< (const T& t); 
		
			/** Logging level setter. */
			void setLogLevel(LogLevel level);
		
			// Singleton releted stuff
			static Logger& getSingleton(void);
			static Logger* getSingletonPtr(void);
	};
	
	// Printf-like logging helping defines
	#define LOG_FATAL(...) Logger::getSingleton().log(LOG_ERROR, __VA_ARGS__)
	#define LOG_ERROR(...) Logger::getSingleton().log(LOG_ERROR, __VA_ARGS__)
	#define LOG_INFO(...) Logger::getSingleton().log(LOG_INFO, __VA_ARGS__)
	
	// the debug+verbose loggers are conditionaly built in when the DEBUG flag is defined
	#ifdef OPDE_DEBUG
		#define LOG_DEBUG(...) Logger::getSingleton().log(LOG_DEBUG, __VA_ARGS__)
		#define LOG_VERBOSE(...) Logger::getSingleton().log(LOG_VERBOSE, __VA_ARGS__)
	#else
		#define LOG_DEBUG(...)
		#define LOG_VERBOSE(...)
	#endif
	
	// Shortcut to the Logger instance
	#define LOG Logger::getSingleton()
}

#endif
