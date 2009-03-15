/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *
 *	  $Id$
 *
 *****************************************************************************/
 
#include "OpdeException.h"

#include <string>
#include <sstream>

using namespace std;

namespace Opde {
	
	
		BasicException::BasicException(const std::string& desc, const std::string& src, const char* file, long line) {
			description = desc;
			source = src;
			
			if (file != NULL) {
				fileName = string(file);
				lineNum = line;
				
				stringstream out;
				out << lineNum;
				
				details = string("Opde Exception (" + fileName + " line " + out.str() + ") - " + source + " : " + description);
			} else {
				fileName = string("-Unsupplied-");
				
				details = string("Opde Exception  - " + source + " : " + description);
			}
			
			
		}
		
		const std::string& BasicException::getDetails() {
			return details;
		}
		
		BasicException::~BasicException() throw() {
		}
}
