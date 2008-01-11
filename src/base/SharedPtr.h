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

#ifndef __SHAREDPTR_H
#define __SHAREDPTR_H

#include <stdio.h>

namespace Opde {

	/** A simple shared pointer implementation
	* @TODO To be made thread-safe, and/or replaced by boosts shared_ptr (which does not have downcasting) */
	template<class T> class shared_ptr {
		template < typename U > friend class shared_ptr;

		protected:
			unsigned int* mReferences;
			T* mPtr;

			void release() {
				if (mReferences) {
					if (--(*mReferences) == 0) {
						destroy();
					}
				}
			}

			void destroy() {
				delete mPtr;
				delete mReferences;

				mPtr = NULL;
				mReferences = NULL;
			}

		public:
			/// NULL ctor
			shared_ptr() : mReferences(NULL), mPtr(NULL) { };

			/// Copy constructor
			shared_ptr(const shared_ptr& b) : mPtr(b.mPtr), mReferences(b.mReferences) {
				if (mReferences)
					++(*mReferences);
			}

			/// Ctor from instance pointer
			shared_ptr(T* ptr) : mPtr(ptr), mReferences(NULL) {
				if (mPtr) {
					mReferences = new unsigned int(1);
				}
			};

			~shared_ptr() {
				release();
			}

			shared_ptr& operator=(const shared_ptr& b) {
				// the same instance
				if (mPtr == b.mPtr)
					return *this;

				release();

				mPtr = b.mPtr;

				mReferences = b.mReferences;

				if (mReferences)
					++(*mReferences);

				return *this;
			}

			/** pointer getter */
			T* ptr() const { return mPtr; };

			inline T& operator*() const { assert(mPtr); return *mPtr; };
			inline T* operator->() const { assert(mPtr); return mPtr; };

			inline bool isNull() const {
				return (mPtr == NULL);
			}

			/// static cast
			template<class U> shared_ptr<U> as() {
				// of course, Dynamic cast would be safer, then again, slower
				U* ptr = static_cast<U*>(mPtr);

				++(*mReferences);

				shared_ptr<U> n = shared_ptr<U>(ptr, mReferences);

				return n;
			}

			unsigned int getRefCount() {
				return (*mReferences);
			}

			void setNull() {
				release();

				mPtr = NULL;
				mReferences = NULL;

			}
			
		private:
			/** Helper ctor for shared_ptr casting with as<U>()
			*/
			shared_ptr(T* ptr, unsigned int *refs) : mPtr(ptr), mReferences(refs) {
			}
	};

	template<class A, class B> inline bool operator==(shared_ptr<A> const& a, shared_ptr<B> const& b) {
		return a.ptr() == b.ptr();
        }

        template<class A, class B> inline bool operator!=(shared_ptr<A> const& a, shared_ptr<B> const& b) {
        	return a.ptr() != b.ptr();
        }

} // namespace Opde

#endif
