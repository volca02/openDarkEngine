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
 *	   $Id$
 *
 *****************************************************************************/


#ifndef __ARRAY_H
#define __ARRAY_H

#include "config.h"
#include "OpdeException.h"

namespace Opde {
	/** A negative index capable, fixed size, growable array. Targetted at
	* per-object ID values.
	* @note Does not support holes - the whole array of stored values is initialized.
	* @note Does not support shrinking - only clear/grow methods can be used
	*/
	template<class T> class Array {
		public:
			/// Constructor. Creates an empty array
			Array() : mMinIndex(1), mMaxIndex(-1),
					  mNegativeArray(NULL), mPositiveArray(NULL) {
				// default size is zero
				mPositiveArray = NULL;
				mNegativeArray = NULL; // only the indices with minus sign are here
			};

			/// copy constructor
			Array(const Array& b) {
				// copy the bounds
				mMinIndex = b.mMinIndex;
				mMaxIndex = b.mMaxIndex;

				mPositiveArray = NULL;
				mNegativeArray = NULL;

				// allocate space
				if (b.mNegativeArray != NULL) {
					mNegativeArray = static_cast<T*>(malloc(-mMinIndex * sizeof(T)));

					// copy construct the elements
					for (int i = 0; i <= -mMinIndex; ++i)
						::new (&mNegativeArray[i]) T(b.mNegativeArray[i]);
				}

				if (b.mPositiveArray != NULL) {
					mPositiveArray = static_cast<T*>(malloc(mMaxIndex * sizeof(T)));

					// copy construct the elements
					for (int i = 0; i <= mMaxIndex; ++i)
						::new (&mPositiveArray[i]) T(b.mPositiveArray[i]);
				}
			}

			/// Destructor. Calls placed destruct on the object in the array
			~Array() {
				clear();
			};

			/// clear method - clears the array - reinitializes it to zero element sized
			void clear() {
				// call placed destructor on the objects
				for (int i = 0; i < -mMinIndex; ++i)
					(&mNegativeArray[i])->~T();

				for (int i = 0; i <= mMaxIndex; ++i)
					(&mPositiveArray[i])->~T();

				// now get rid of the arrays
				free(mNegativeArray);
				free(mPositiveArray);

				mNegativeArray = NULL;
				mPositiveArray = NULL;

				mMinIndex = 1;
				mMaxIndex = -1;
			}

			/// member const reference array operator
			const T& operator[](int index) const
			{
				if(index < 0)
				{
					if (index+1 < mMinIndex)
						OPDE_ARRAY_EXCEPT("Array: Index out of bounds");

					return (mNegativeArray[-index - 1]);
				}
				else
				{
					if (index > mMaxIndex)
						OPDE_ARRAY_EXCEPT("Array: Index out of bounds");

					return (mPositiveArray[index]);
				}
			}

			/// member reference array operator
			T& operator[](int index)
			{
				if(index < 0)
				{
					if (index+1 < mMinIndex)
						OPDE_ARRAY_EXCEPT("Array: Index out of bounds");

					return (mNegativeArray[-index - 1]);
				}
				else
				{
					if (index > mMaxIndex)
						OPDE_ARRAY_EXCEPT("Array: Index out of bounds");

					return (mPositiveArray[index]);
				}
			}

			/// Grows the array. New sizes have to be greater the old - so no objects are removed
			void grow(int minIdx, int maxIdx) {
				if (minIdx > 0)
					OPDE_ARRAY_EXCEPT("Array: Min index has to be equal to zero or less");

				if (maxIdx < 0)
					OPDE_ARRAY_EXCEPT("Array: Max index has to be greater or equal to zero");

				// the only single place we have to increment - negative array does NOT store element of index zero
				minIdx++;

				growBuf(&mNegativeArray, -mMinIndex, -minIdx);
				mMinIndex = minIdx;

				growBuf(&mPositiveArray, mMaxIndex, maxIdx);
				mMaxIndex = maxIdx;
			}

			int getMinIndex() { return mMinIndex; };
			int getMaxIndex() { return mMaxIndex; };

		protected:
			void growBuf(T**ptr, int oldSize, int newSize) {
				if (newSize < oldSize) // if it would, we'd call placement destructor before realloc
					OPDE_ARRAY_EXCEPT("Array: Shrinking not allowed");

				// just to be sure
				if (oldSize < 0)
                    oldSize = 0;

				if (newSize == oldSize)
					return;

				T* newptr = (T*)(realloc(*ptr, sizeof(T) * newSize));

				if (newptr == NULL) // realloc failed
					OPDE_ARRAY_EXCEPT("Array: Growth failed");

				*ptr = newptr;

                assert(oldSize >= 0);
                assert(newSize >= 0);

				// the damn VC++ does not initialize the contents for primitive types it seems
				memset((*ptr) + oldSize, 0, sizeof(T) * (newSize - oldSize));

				//placement new on the new part of array
				::new((*ptr) + oldSize) T[newSize - oldSize];
			}

			int mMinIndex;
			int mMaxIndex;

			/// Stores the negative part of the array
			T* mNegativeArray;

			/// Stores the positive part of the array
			T* mPositiveArray;
	};

	/** A  fixed size, reallocatable array.
		* @note Does not support holes - the whole array of stored values is initialized.
		* @note Does not support shrinking - only clear/grow methods can be used
		*/
		template<class T> class SimpleArray {
			public:
				/// Constructor. Creates an empty array
				SimpleArray() : mSize(0), mArray(NULL) {
				};

				/// copy constructor
				SimpleArray(SimpleArray& b) {
					// copy the bounds
					mSize = b.mSize;

					mArray = NULL;

					// allocate space
					if (b.mArray != NULL) {
						mArray = static_cast<T*>(malloc(mSize * sizeof(T)));

						// copy construct the elements
						for (size_t i = 0; i < mSize; ++i)
							::new (&mArray[i]) T(b.mArray[i]);
					}
				}

				/// Destructor. Calls placed destruct on the object in the array
				~SimpleArray() {
					clear();
				};

				/// clear method - clears the array - reinitializes it to zero element sized
				void clear() {
					// call placed destructor on the objects
					for (size_t i = 0; i < mSize; ++i)
						(&mArray[i])->~T();

					// now get rid of the arrays
					free(mArray);

					mArray = NULL;

					mSize = 0;
				}

				/// member const reference array operator
				const T& operator[](size_t index) const
				{
					if (index >= mSize)
						OPDE_ARRAY_EXCEPT("SimpleArray: Index out of bounds");

					return (mArray[index]);
				}

				/// member reference array operator
				T& operator[](size_t index)
				{
					if (index >= mSize)
						OPDE_ARRAY_EXCEPT("SimpleArray: Index out of bounds");

					return (mArray[index]);
				}

				/// Grows the array. New sizes have to be greater the old - so no objects are removed
				void grow(size_t newSize) {
					growBuf(newSize);
				}

				size_t getSize() { return mSize; };

			protected:
				void growBuf(size_t newSize) {
					if (newSize < mSize)
						OPDE_ARRAY_EXCEPT("SimpleArray: Shrinking not allowed");

					if (newSize == mSize)
						return;

					T* newptr = (T*)(realloc(mArray, sizeof(T) * newSize));

					if (newptr == NULL) // realloc failed
						OPDE_ARRAY_EXCEPT("SimpleArray: Growth failed");

					mArray = newptr;

					//placement new on the new part of array
					::new(mArray + mSize) T[newSize - mSize];

					mSize = newSize;
				}

				/// Stores the size of the array
				size_t mSize;

				/// Stores the array itself
				T* mArray;
		};

}


#endif
