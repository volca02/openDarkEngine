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
 
#ifndef logging_h
#define logging_h

#ifdef _MSC_VER 
#pragma warning( disable : 4996 )
#endif

//////////////////// logging mumbo - jumbo
const char* LOG_LEVEL_STRINGS[5] = {"FATAL","ERROR","INFO ","DEBUG", "ALL  "};

#define LOG_VERBOSE 4
#define LOG_DEBUG 3
#define LOG_INFO 2
#define LOG_ERROR 1
#define LOG_FATAL 0

long loglevel = LOG_ERROR;

void log(long level, const char *fmt, ...) {
	if (level > loglevel)
		return;
	
	// to filter out those which do not fit in the table of strings describing level
	if (level > LOG_VERBOSE)
		level = LOG_VERBOSE;
	if (level < 0)
		level = 0;
	
	va_list argptr;
	char result[255];
	
	va_start(argptr, fmt);
	vsnprintf(result, 255, fmt, argptr);
	va_end(argptr);
	
	printf("Log (%s) : %s\n", LOG_LEVEL_STRINGS[level], result);
}

#define log_fatal(...) log(LOG_ERROR, __VA_ARGS__)
#define log_error(...) log(LOG_ERROR, __VA_ARGS__)
#define log_info(...) log(LOG_INFO, __VA_ARGS__)
#define log_debug(...) log(LOG_DEBUG, __VA_ARGS__)
#define log_verbose(...) log(LOG_VERBOSE, __VA_ARGS__)

#endif
