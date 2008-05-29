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

#include "OpdeException.h"

namespace Opde {
	class IndexOutOfBoundsException : public BasicException {
		public:
			IndexOutOfBoundsException(const std::string& txt, char* file = NULL, long line = -1)
				: BasicException(txt, "", file, line) {};
	};
	
	#define OPDE_BIT_ARRAY_EXCEPT(txt) throw( Opde::IndexOutOfBoundsException(txt, __FILE__, __LINE__) )
	
	/** Bit array supporting negative and positive indices. Maps a single boolean value */
	class BitArray {
		public:
			/// constructs a new empty bitarray
			BitArray() : mArray(NULL), mMinIDX(0), mMaxIDX(-1), mByteSize(0) {};
			
			/// constructs a new bitarray with a specified min and max boundaries (all values false)
			BitArray(int min, int max) : mMinIDX(min), mMaxIDX(max) {
				mArray = prepareBuffer(mMinIDX, mMaxIDX, mByteSize);
			}
			
			/// constructs a bitarray as a copy of a source buffer of a given size
			BitArray(unsigned char* source, int size, int min, int max) {
				assert(max > min);
				
				// the source can be non-aligned. transfer the bits
				
				mMinIDX = min;
				mMaxIDX = max;
				
				assert((max - min) >> 3 <= size);

				mArray = prepareBuffer(mMinIDX, mMaxIDX, mByteSize);
				
				for (int idx = min; idx <= max; ++idx) {
					// transfer the id
					size_t pos = idx - min;
					
					bool val = ((source[pos >> 3] & (1 << (pos & 0x07))) != 0);
					set(idx, val);
				}
			};
			
			~BitArray() {
				delete[] mArray;
			}
						
			bool get(int index) const {
				// look if we are in bounds
				if (index < mMinIDX)
					OPDE_BIT_ARRAY_EXCEPT("Index out of bounds");
					
				if (index > mMaxIDX)
					OPDE_BIT_ARRAY_EXCEPT("Index out of bounds");
					
				// remap the id
				size_t cpos = (index - mMinIDX) >> 3;
				size_t bpos = (index - mMinIDX) & 0x07;
				
				// get the value
				return ((mArray[cpos] & (1 << bpos)) != 0);
			};
			
			/// sets a new value for index, returns the old value
			bool set(int index, bool value) {
				// look if we are in bounds
				if (index < mMinIDX)
					OPDE_BIT_ARRAY_EXCEPT("Index out of bounds");
					
				if (index > mMaxIDX)
					OPDE_BIT_ARRAY_EXCEPT("Index out of bounds");				
					
				// remap the id
				size_t cpos = (index - mMinIDX) >> 3;
				unsigned char bpos = (index - mMinIDX) & 0x07;
				
				// get the value
				bool prev = ((mArray[cpos] & (1 << bpos)) != 0);
				
				// set the value
				if (value) {
					mArray[cpos] |= (1 << bpos);
				} else {
					// unset the bit
					mArray[cpos] &= ~(1 << bpos);
				}

				return prev;
			};
			
			int getMinIndex() const { return mMinIDX; };
			int getMaxIndex() const { return mMaxIDX; };

			/** sets new min index
			* @param newidx The new minimal index 
			* @param clear - if true, the min index can be greater than the old one, and the bitarray is cleared as well 
			* @note if the new minimum is greater than the old, and clear is not set, the new value is ignored
			*/
			void setMinIndex(int newidx, bool clear) {
				if (clear) {
					delete[] mArray;
					
					mMinIDX = newidx;
					
					mArray = prepareBuffer(mMinIDX, mMaxIDX, mByteSize);
				} else {
					if (newidx < mMinIDX)
						grow(newidx, mMaxIDX);
				}
			};
			
			/** sets new max index
			* @param newidx The new maximal index 
			* @param clear - if true, the max index can be less than the old one, and the bitarray is cleared as well 
			* @note if the new maximum is less than the old, and clear is not set, the new value is ignored
			*/
			void setMaxIndex(int newidx, bool clear) {
				if (clear) {
					delete[] mArray;
					
					mMaxIDX = newidx;
					
					mArray = prepareBuffer(mMinIDX, mMaxIDX, mByteSize);
				} else {
					if (newidx > mMaxIDX)
						grow(mMinIDX, newidx);
				}
			};

			
			const unsigned char* getRawBuf() const { return mArray; };
			size_t getRawBufSize() const { return mByteSize; };
			
			/// clears all true values in bitarray, leaving the size intact
			void clear() { 
				memset(mArray, 0, mByteSize);
			};
			
			/// clears the array and sets new min and max boundary values
			void reset(int min, int max) {
				mMinIDX = min;
				mMaxIDX = max;
				
				delete[] mArray;
				
				mArray = prepareBuffer(mMinIDX, mMaxIDX, mByteSize);
			};
			
			/// grows the bitArray to be able to index new min->max indices, preserving the previous values
			void grow(int min, int max) {
				int oldMin = mMinIDX, oldMax = mMaxIDX;
				bool changed = false;
				
				// new bounds are calculated
				if (min < mMinIDX) {
					mMinIDX = min;
					changed = true;
				}
					
				if (max > mMaxIDX) {
					mMaxIDX = max;
					changed = true;
				}
				
				if (changed) {
					size_t size;
					
					// prepare the new bit buffer. This'll also correct the boundaries
					unsigned char* newbuf = prepareBuffer(mMinIDX, mMaxIDX, size);
					
					// transfer the values from the original...
					if (mArray) {
						// reusing the var err here for byte aligned min diff
						int err = oldMin - mMinIDX; // mMinIDX < oldMin -> err > 0
						
						LOG_VERBOSE("BitArray growth: (%d-%d) to (%d-%d). Diff: %d (%d)", oldMin, oldMax, mMinIDX, mMaxIDX, err, err & 0x07);
						
						assert((err & 0x07) == 0);
						
						err >>= 3;
						
						// copy the prev values
						memcpy(mArray, &newbuf[err], mByteSize);
						
						// there, the old values are copied
					}
					
					mByteSize = size;
					
					delete[] mArray;
					mArray = newbuf;
				}
			};
			
		protected:
			/// prepares a new buffer with the specified min and max values, and corrects those as a side effect
			unsigned char* prepareBuffer(int& min, int& max, size_t& byteSize) {
				assert(min < max);
				
				// allocate and clear the mem
				byteSize = max - min;
					
				// align the new minimum
				int err = byteSize & 0x07;
				
				if (err != 0) {
					min -= (8 - err);
					byteSize = max - min;
				}
					

				if ((byteSize & 0x07) != 0) {
					// alignment of size needed
					byteSize += (8 - (byteSize & 0x07));
				}
				
				// to the byte world
				byteSize >>= 3;
				byteSize++; // safety first
				
				unsigned char *array = new unsigned char[byteSize];
				
				memset(array, 0, byteSize);
				
				return array;
			}
		
			unsigned char* mArray;
			size_t mByteSize;
			int mMinIDX, mMaxIDX;
	};
};

#endif // __BITARRAY_H
