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

#ifndef __SHAREDPTR_H
#define __SHAREDPTR_H

#include "config.h"

#include <stdio.h>
#include <cassert>

namespace Opde {

	/** A simple shared pointer implementation
	* @todo To be made thread-safe, and/or replaced by boosts shared_ptr (which does not have downcasting) */
	template<class T> class shared_ptr {
		template < typename U > friend class shared_ptr;

		protected:
			T* mPtr;
			unsigned int* mReferences;

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
			/** Helper ctor for shared_ptr casting with static_pointer_cast<U>().
			* @warning Do not use directly */
			shared_ptr(T* ptr, unsigned int *refs) : mPtr(ptr), mReferences(refs) {
				++(*mReferences);
			}

			/// conversion ctor
			template<class U> shared_ptr(shared_ptr<U>& a) : mPtr(a.mPtr), mReferences(a.mReferences) {
				if (mReferences)
					++(*mReferences);
			}

			/// NULL ctor
			shared_ptr() : mPtr(NULL), mReferences(NULL) { };

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

			unsigned int getRefCount() const {
				return (*mReferences);
			}

			void setNull() {
				release();

				mPtr = NULL;
				mReferences = NULL;

			}

			unsigned int* getRefCountPtr(void) const {
				return mReferences;
			}

	};

	template<class A, class B> inline bool operator==(shared_ptr<A> const& a, shared_ptr<B> const& b) {
		return a.ptr() == b.ptr();
	}

    template<class A, class B> inline bool operator!=(shared_ptr<A> const& a, shared_ptr<B> const& b) {
      	return a.ptr() != b.ptr();
    }

	/// static cast of the shared_ptr
	template<class U, class V> shared_ptr<U> static_pointer_cast(const shared_ptr<V>& src) {
		U* ptr = static_cast<U*>(src.ptr());

		return shared_ptr<U>(ptr, src.getRefCountPtr());
	}

} // namespace Opde

#endif
