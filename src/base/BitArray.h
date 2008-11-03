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
 *	$Id$
 *
 *****************************************************************************/

#ifndef __BITARRAY_H
#define __BITARRAY_H

#include "config.h"
#include <cassert>

#include "OpdeException.h"

namespace Opde {
	/** Bit array supporting negative and positive indices. Maps a single boolean value */
	class BitArray {
		public:
			/// constructs a new empty bitarray
			BitArray() : mNegativeArray(NULL), mPositiveArray(NULL), mMinIndex(1), mMaxIndex(-1) {};
			
			/// constructs a new bitarray with a specified min and max boundaries (all values false)
			explicit BitArray(int min, int max) : mNegativeArray(NULL), mPositiveArray(NULL), mMinIndex(1), mMaxIndex(-1) {
				grow(min, max);
			}
			
			/// copy constructor
			BitArray(const BitArray& b) {
				// copy the bounds
				mMinIndex = b.mMinIndex;
				mMaxIndex = b.mMaxIndex;
				
				mPositiveArray = NULL;
				mNegativeArray = NULL;
				
				// allocate space
				if (b.mNegativeArray != NULL) {
					size_t bsize = getSizeFromIndex(-mMinIndex);
					mNegativeArray = (unsigned char*)(malloc(bsize));
					// copy the values
					memcpy(mNegativeArray, b.mNegativeArray, bsize);
				}
				
				if (b.mPositiveArray != NULL) {
					size_t bsize = getSizeFromIndex(mMaxIndex);
					mPositiveArray = (unsigned char*)(malloc(bsize));
					// copy the values
					memcpy(mPositiveArray, b.mPositiveArray, bsize);
				}
			}
			
			/// constructs a bitarray as a copy of a source buffer of a given size
			BitArray(unsigned char* source, int size, int min, int max)
				: mNegativeArray(NULL), mPositiveArray(NULL), mMinIndex(1), mMaxIndex(-1) {
				
				assert(max > min);
				
				// sanity check
				assert((max - min) >> 3 <= size);

				// prepare
				grow(min, max);
				
				// as we can have a different bit alignment, do this bit by bit
				for (int idx = min; idx <= max; ++idx) {
					// transfer the bits
					size_t pos = idx - min;
					
					bool val = ((source[pos >> 3] & (1 << (pos & 0x07))) != 0);
					setBit(idx, val);
				}
			};
			
			~BitArray() {
				clear();
			}
				
			/// clear method - clears the array - reinitializes it to zero element sized
			void clear() {
				// now get rid of the arrays
				free(mNegativeArray);
				free(mPositiveArray);
				
				mNegativeArray = NULL;
				mPositiveArray = NULL;
				
				mMinIndex = 0;
				mMaxIndex = -1;
			}
			
			bool getBit(int index) const {
				// look if we are in bounds
				if (index < mMinIndex)
					OPDE_ARRAY_EXCEPT("Index out of bounds");
					
				if (index > mMaxIndex)
					OPDE_ARRAY_EXCEPT("Index out of bounds");
					
				// depending on the index sign, we either access negative
				// or positive array
				unsigned char* arrayRef;
				
				// character and bit positions
				size_t cpos;
				size_t bpos;
				
				if (index < 0) {
					arrayRef = mNegativeArray;
					cpos = getIndex(-index - 1);
					bpos = getOffset(-index - 1);
				} else {
					arrayRef = mPositiveArray;
					cpos = getIndex(index);
					bpos = getOffset(index);
				}
				
				
				// get the value
				return ((arrayRef[cpos] & (1 << bpos)) != 0);
			};
			
			/// sets a new value for index, returns the old value
			bool setBit(int index, bool value) {
				// look if we are in bounds
				if (index < mMinIndex)
					OPDE_ARRAY_EXCEPT("BitArray: Index out of bounds");
					
				if (index > mMaxIndex)
					OPDE_ARRAY_EXCEPT("BitArray: Index out of bounds");
					
				// remap the id
				unsigned char* arrayRef;
				
				// character and bit positions
				size_t cpos;
				size_t bpos;
				
				if (index < 0) {
					arrayRef = mNegativeArray;
					cpos = getIndex(-index - 1);
					bpos = getOffset(-index - 1);
				} else {
					arrayRef = mPositiveArray;
					cpos = getIndex(index);
					bpos = getOffset(index);
				}
				
				// get the value
				bool prev = ((arrayRef[cpos] & (1 << bpos)) != 0);
				
				// set the value
				if (value) {
					arrayRef[cpos] |= (1 << bpos);
				} else {
					// unset the bit
					arrayRef[cpos] &= ~(1 << bpos);
				}

				return prev;
			};
			
			int getMinIndex() const { return mMinIndex; };
			int getMaxIndex() const { return mMaxIndex; };

			/// returns a byte size needed to hold the whole buffer
			size_t getByteSize() const {
				return getSizeFromIndex(-mMinIndex-1) + getSizeFromIndex(mMaxIndex);
			};
			
			/** fills a given buffer with bits from this bit array. 
			* The given buffer has to be allocated so it can hold up getByteSize() bytes
			*/
			void fillBuffer(char* buffer) const { 
				// sane default
				memset(buffer, 0, getByteSize());
				
				size_t bitoffset = 0;
				for (int i = mMinIndex; i <= mMaxIndex; ++i, ++bitoffset) {
					// each bit on it's own
					bool val = getBit(i);
					
					// i know this sucks to do on every bit, but well
					// I Am lazy at the same time :)
					if (val)
						buffer[bitoffset>>3] |= (1 << (bitoffset & 0x07));
				}
			};
			
			/// clears the array values, setting all bits to false
			void clearBits() { 
				memset(mNegativeArray, 0, getSizeFromIndex(-mMinIndex));
				memset(mPositiveArray, 0, getSizeFromIndex(mMaxIndex));
			};
			
			/// clears the array and sets new min and max boundary values
			void reset(int min, int max) {
				clear();
				grow(min, max);
			};
			
			/// grows the bitArray to be able to index new min->max indices, preserving the previous values
			void grow(int min, int max) {
				growMinIndex(min);
				growMaxIndex(max);
			};
			
			/// grows the min index part only
			void growMinIndex(int min) {
				if (min > 0)
					OPDE_ARRAY_EXCEPT("BitArray: Min index has to be equal to zero or less");
				
				growBuf(&mNegativeArray, -mMinIndex, -min);
				mMinIndex = min;
			}
			
			/// grows the max index part only
			void growMaxIndex(int max) {
				if (max < 0)
					OPDE_ARRAY_EXCEPT("BitArray: Max index has to be greater or equal to zero");
				
				growBuf(&mPositiveArray, mMaxIndex, max);
				mMaxIndex = max;
			}
			
			/// proxy for value setting with [] operator
			class BitProxy {
				public:
					BitProxy(BitArray& array, int index) : mArray(array), mIndex(index) {	};
					
					BitProxy& operator =(BitProxy& b) {
						mArray.setBit(mIndex, b.operator bool());
						return *this;
					}
					
					BitProxy& operator =(bool value) {
						mArray.setBit(mIndex, value);
						
						return *this;
					};
					
					operator bool() const {
						return mArray.getBit(mIndex);
					}
				
				protected:
					BitArray& mArray;
					int mIndex;
			};
			
			BitProxy operator[](int index) {
				return BitProxy(*this, index);
			};
			
			bool operator[](int index) const {
				return getBit(index);
			};
			
		protected:
			/// grows the specified buffer to accompany the new size
			void growBuf(unsigned char**ptr, int oldIndex, int newIndex) {
				if (newIndex < oldIndex)
					OPDE_ARRAY_EXCEPT("BitArray: Shrinking not allowed");
				
				size_t oldByteSize;
				if (oldIndex < 0)
					oldByteSize = 0;
				else
				   oldByteSize = getSizeFromIndex(oldIndex);
				   
				size_t newByteSize = getSizeFromIndex(newIndex);
				
				if (newByteSize == oldByteSize)
					return;
					
				
				unsigned char* newptr = (unsigned char*)(realloc(*ptr, newByteSize));
				
				if (newptr == NULL) // realloc failed
					OPDE_ARRAY_EXCEPT("BitArray: Growth failed");
				
				*ptr = newptr;
				
				memset((*ptr) + oldByteSize, 0, newByteSize - oldByteSize);
			}
		
			/// byte offset of an address getter
			inline size_t getIndex(int addr) const {
				return size_t(addr) >> 3;
			}
			
			/// Bit offset for address getter
			inline int getOffset(int addr) const {
				return size_t(addr) & 0x07;
			}
			
			/// Helper size getter for index - needed byte size for array to hold certain index
			inline size_t getSizeFromIndex(int index) const {
				assert(index >= 0);
				
				return size_t(getIndex(index)) + 1;
			}
			
			unsigned char* mNegativeArray;
			unsigned char* mPositiveArray;
			
			int mMinIndex, mMaxIndex;
	};
	
	
};

#endif // __BITARRAY_H
