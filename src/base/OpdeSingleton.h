/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *****************************************************************************/
 
// A copy of Ogre::Singleton. LGPL

#ifndef __OPDESINGLETON_H
#define __OPDESINGLETON_H

#include "config.h"

#include <cassert>

namespace Opde {
	
	// Just a copy of Ogre's singleton impl, before writing something on our own. Thanks OGRE people!
    template <typename T> class Singleton {
	    protected:
	        static T* ms_Singleton;

	    public:
    		Singleton( void )  {
        	    assert( !ms_Singleton );
    		#if defined( _MSC_VER ) && _MSC_VER < 1200
	            int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
	            ms_Singleton = (T*)((int)this + offset);
			#else
	            ms_Singleton = static_cast< T* >( this );
			#endif
        	}

            ~Singleton( void ) {
				assert( ms_Singleton );
				ms_Singleton = 0;
			}

			/* Let the ancestor implement these methods. They are dangerous here because of the DLL boundaries
			static T& getSingleton( void ) {
				assert( ms_Singleton );  
				return ( *ms_Singleton );
			}

			static T* getSingletonPtr( void ) {
				return ms_Singleton;
			}
			*/
    };

}

#endif
