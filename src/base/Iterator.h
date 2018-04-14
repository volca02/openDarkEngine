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

#ifndef __ITERATOR_H
#define __ITERATOR_H

#include "config.h"

namespace Opde {

    /** A java-like iterator approach */
    template <typename T> class Iterator {
	public:
	    virtual ~Iterator(void) {};
	    virtual T& next() = 0;
	    virtual bool end() const = 0;
    };


    /** A const, java-like iterator approach */
    template <typename T> class ConstIterator {
	public:
        virtual ~ConstIterator(void) {};
	    virtual const T& next() = 0;
	    virtual bool end() const = 0;
    };

  	/// Class representing a const string iterator
	typedef ConstIterator< std::string > StringIterator;

	/// Shared pointer instance to const string iterator
	typedef std::shared_ptr< StringIterator > StringIteratorPtr;

	/// Class representing a const int iterator
	typedef ConstIterator< int > IntIterator;

	/// Shared pointer instance to const int iterator
	typedef std::shared_ptr< IntIterator > IntIteratorPtr;

	/// Map key iterator
	template<class C, typename T> class MapKeyIterator : public ConstIterator<T> {
		public:
			MapKeyIterator(const C& map) : mMap(map) {
				mIter = mMap.begin();
			}

			const T& next() {
				assert(!end());

				const int& val = mIter->first;

				++mIter;

				return val;
			};

			bool end() const {
				return mIter == mMap.end();
			};

		protected:
			const C& mMap;
			typename C::const_iterator mIter;
	};
}

#endif
