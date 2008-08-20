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
 *	  $Id$
 *****************************************************************************/

#include <OgreLogManager.h>
#include "config.h"
#include "File.h"

using namespace std;
using namespace Ogre;

namespace Opde {

	/*------------------------------------------------------*/
	/*------------------------- File -----------------------*/
	/*------------------------------------------------------*/
	File::File(const std::string& name, AccessMode mode) : mFileName(name), mAccessMode(mode) {
	}

	//------------------------------------
	File::~File() {
	}

	//------------------------------------
	void File::writeToFile(File& dest) {
		file_size_t tsz = size();

		file_size_t _pos = tell();

		seek(0);

		while (tsz > 0) {
			char temp[MEMORY_FILE_BUF_LEN];

			file_size_t sz = (tsz > MEMORY_FILE_BUF_LEN) ? MEMORY_FILE_BUF_LEN : tsz;

			read(temp, sz);
			dest.write(temp, sz);

			tsz -= sz;
		}

		// go to the previous position
		seek(_pos);
	}

	//------------------------------------
	File& File::readElem(void* buf, file_size_t size, uint count) {
		read(buf, size * count);

#ifdef __OPDE_BIG_ENDIAN
		swapEndian(buf, size, count);
#endif
		return *this;
	}

	//------------------------------------
	File& File::writeElem(const void* buf, file_size_t size, uint count) {
#ifdef __OPDE_BIG_ENDIAN
		file_size_t bsize = size * count;
		char *copyb = new char[bsize];

		memcpy(copyb, buf, bsize);

		swapEndian(copyb, size, count);

		write(copyb, bsize);

		delete[] copyb;
#else
		write(buf, size * count);
#endif

		return *this;
	}

	//------------------------------------
	File& File::readStruct(void* buf, const char* format, uint32_t ExpectedLen, uint count) {
		uint32_t TotalBytes = 0;		
		for (uint i = 0; i < count; i++) {
			uint32_t Index = 0;
			char* bpos = static_cast<char*>(buf);
		    
			while (format[Index]) {
				int rep = 0;
				bool haverep = false;
			
				// While the char is a number, compose an integer out of it
				while (format[Index] >= '0' && format[Index] <= '9') {
					rep *= 10;
					rep += format[Index] - '0';
			    
					Index++;
					
					haverep = true; // A repetitions count was supplied
				}
			
				if (!haverep) // no repetitions supplied
					rep = 1;
			
				size_t elem_size = 0;
				
				if (!format[Index]) // End of line after repetitions? That is not format compliant!
					OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Element count should be followed by format!", "File::readStruct");
				
				switch (format[Index]) {
					case 'c' : elem_size = 1; break;
					case 'w' : elem_size = 2; break;
					case 'i' :
					case 'f' : elem_size = 4; break;
					case 'q' : elem_size = 8; break;
					
					default:
						OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Bad readStruct format!", "File::readStruct");
				}
				
				// Go to next char, would you kindly? ;)
				Index++;
				
				if (elem_size > 1)
				    readElem(bpos, elem_size, rep);
				else
				    read(bpos, rep);
				
				// Increment the position
				bpos += elem_size * rep;
				TotalBytes += elem_size * rep;
			}
		}		
		if(TotalBytes != ExpectedLen)
			LogManager::getSingleton().logMessage("readStruct warning: ExpectedLen != TotalBytes");
		return *this;
	}
	
	//------------------------------------
	File& File::writeStruct(const void* buf, const char* format, uint32_t ExpectedLen, uint count) {
		uint32_t TotalBytes = 0;
		for (uint i = 0; i < count; i++) {
			uint32_t Index = 0;
			const char* bpos = static_cast<const char*>(buf);
		    
			while (format[Index]) {
				int rep = 0;
				bool haverep = false;
			
				// While the char is a number, compose an integer out of it
				while (format[Index] >= '0' && format[Index] <= '9') {
					rep *= 10;
					rep += format[Index] - '0';
			    
					Index++;
					
					haverep = true; // A repetitions count was supplied
				}
			
				if (!haverep) // no repetitions supplied
					rep = 1;
			
				size_t elem_size = 0;
				
				if (!format[Index]) // End of line after repetitions? That is not format compliant!
					OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Element count should be followed by format!", "File::readStruct");
				
				switch (format[Index]) {
					case 'c' : elem_size = 1; break;
					case 'w' : elem_size = 2; break;
					case 'i' :
					case 'f' : elem_size = 4; break;
					case 'q' : elem_size = 8; break;
					
					default:
						OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Bad writeStruct format!", "File::writeStruct");
				}
				
				// Go to next char, would you kindly? ;)
				Index++;
				

				if (elem_size > 1)
				    writeElem(bpos, elem_size, rep);
				else
				    write(bpos, rep);
				
				// Increment the position
				bpos += elem_size * rep;
				TotalBytes += elem_size * rep;
			}
		}
		if(TotalBytes != ExpectedLen)
			LogManager::getSingleton().logMessage("writeStruct warning: ExpectedLen != TotalBytes");

		return *this;
	}

	//------------------------------------
	void File::swapEndian(void* buf, file_size_t size, uint count) {
		// swap all elements
		for (uint i = 0; i < count; i++) {

			char sw;
			// swap the element's bytes
			for (uint o = 0; o < size/2; o++) {
				sw = *(((char*) buf) + o);

				*(((char*) buf) + o) = *(((char*) buf) + size - o - 1);
				*(((char*) buf) + size - o - 1) = sw;
			}
		}
	}
	
	/*------------------------------------------------------*/
	/*---------------------- StdFile -----------------------*/
	/*------------------------------------------------------*/
	StdFile::StdFile(const std::string& name, AccessMode mode) : File(name, mode), mStream() {
		switch (mAccessMode) {
			case FILE_R  :
				mStream.open(mFileName.c_str(), ios::in  | ios::binary);
				break;
			case FILE_RW :
				mStream.open(mFileName.c_str(), ios::in  | ios::out | ios::binary);
				break;
			case FILE_W  :
				mStream.open(mFileName.c_str(), ios::out | ios::binary);
				break;

			default:
				OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Unknown open mode", "StdFile::StdFile()");
		}



		if (!mStream.is_open()) // open failed
			OPDE_FILEEXCEPT(FILE_OPEN_ERROR, string("File open failed for '") + mFileName + "'", "StdFile::StdFile()");
	}

	//------------------------------------
	StdFile::~StdFile() {
		mStream.close();
	}

	//------------------------------------
	const file_size_t StdFile::size() {
		OPDE_FILEEXCEPT(FILE_UNIMPL, "size method not implemented for StdFile", "StdFile.size()");
	}

	//------------------------------------
	void StdFile::seek(file_offset_t pos, SeekMode mode) {
		ios_base::seekdir dir;

		switch (mode) {
			case FSEEK_BEG :
				dir = ios_base::beg;
				break;
			case FSEEK_END :
				dir = ios_base::end;
				break;
			case FSEEK_CUR :
				dir = ios_base::cur;
				break;

			default:  // should not happen
				OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Unknown seek position modifier", "StdFile.seek()");
		}

		mStream.seekg(pos, dir);

		// detect the seek status
		if (mStream.fail())
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Seek failed", "StdFile.seek()");
	}

	//------------------------------------
	void StdFile::seek(file_pos_t pos) {
		mStream.seekg(pos);

		// detect the seek status
		if (mStream.fail())
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Seek failed", "StdFile.seek()");
	}

	//------------------------------------
	const file_pos_t StdFile::tell() {
		return mStream.tellg();
	}

	//------------------------------------
	File& StdFile::read(void* buf, file_size_t size) {
		if (!isReadable())
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Read on write-only file", "StdFile.read()");

		mStream.read(static_cast<char*>(buf), size);

		if (mStream.fail()) {
			mStream.clear(); // Clear the status flags
			OPDE_FILEEXCEPT(FILE_READ_ERROR, "Read operation failed", "StdFile.read()");
		}

		return *this;
	}

	//------------------------------------
	File& StdFile::write(const void* buf, file_size_t size) {
		if (!isWriteable())
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Write on read-only file", "StdFile.write()");

		mStream.write(static_cast<const char*>(buf), size);

		if (mStream.fail()) {
			mStream.clear(); // Clear the status flags
			OPDE_FILEEXCEPT(FILE_READ_ERROR, "Write operation failed", "StdFile.write()");
		}

		return *this;
	}

	//------------------------------------
	bool StdFile::eof() const {
		return mStream.eof();
	}

	//------------------------------------
	std::fstream& StdFile::getStream() {
		return mStream;
	}

	/*------------------------------------------------------*/
	/*--------------------- OgreFile -----------------------*/
	/*------------------------------------------------------*/
	OgreFile::OgreFile(const Ogre::DataStreamPtr& stream) : File(stream->getName(), File::FILE_R), mStream(stream) {
		mAccessMode = FILE_R;
	}

	const file_size_t OgreFile::size() {
		return mStream->size();
	}

	void OgreFile::seek(file_offset_t pos, SeekMode mode) {
		file_pos_t npos = pos;

		switch (mode) {
			case FSEEK_BEG : break;
			case FSEEK_END :
				npos = mStream->size()  - pos;
				break;
			case FSEEK_CUR :
				npos += mStream->tell();
				break;
			default:  // should not happen
				OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Unknown seek position modifier", "OgreFile.seek()");
		}

		mStream->seek(npos);
	};

	void OgreFile::seek(file_pos_t pos) {
		mStream->seek(pos);
	};

	const file_pos_t OgreFile::tell() {
		return mStream->tell();
	};

	File& OgreFile::read(void* buf, file_size_t size) {
		mStream->read(buf, size);

		return *this;
	};

	File& OgreFile::write(const void* buf, file_size_t size) {
		OPDE_FILEEXCEPT(FILE_WRITE_ERROR, "OgreFile write is not possible", "OgreFile::write");
	};

	bool OgreFile::eof() const {
		return mStream->eof();
	}

	/*------------------------------------------------------*/
	/*--------------------- MemoryFile ---------------------*/
	/*------------------------------------------------------*/
	MemoryFile::MemoryFile(const std::string& name, AccessMode mode) : File(name, mode), mPages(),
	 		mSize(0), mEof(true), mFilePos(0) {
	}

	//------------------------------------
	MemoryFile::~MemoryFile() {
		FilePages::iterator it = mPages.begin();

		for (;it != mPages.end(); it++) {
			delete[] (*it); // Release the allocated buf.
		}

		mPages.clear();
	}

	//------------------------------------
	std::pair<size_t, int> MemoryFile::decomposePos(file_pos_t pos) {
		size_t page;
		int pgpos;

		page  = pos / MEMORY_FILE_BUF_LEN;
		pgpos = pos % MEMORY_FILE_BUF_LEN;

		return std::pair<size_t, int>(page, pgpos);
	}

	//------------------------------------
	const file_size_t MemoryFile::size() {
		return mSize;
	}

	//------------------------------------
	void MemoryFile::seek(file_offset_t pos, SeekMode mode) {
		file_pos_t npos = mFilePos; // new position

		switch (mode) {
			case FSEEK_BEG :
				npos = pos;
				break;
			case FSEEK_END :
				npos = mSize - pos;
				break;
			case FSEEK_CUR :
				npos += pos;
				break;

			default:  // should not happen
				OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Unknown seek position modifier", "MemoryFile.seek()");
		}

		if (npos > mSize)
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Resulting position not within the file size", "MemoryFile::seek()");

		mFilePos = npos;

		mEof = (mFilePos >= mSize);
	}

	//------------------------------------
	void MemoryFile::seek(file_pos_t pos) {
		seek(pos, FSEEK_BEG);
	}

	//------------------------------------
	const file_pos_t MemoryFile::tell() {
		return mFilePos;
	}

	//------------------------------------
	File& MemoryFile::read(void* buf, file_size_t size) {
		if (!isReadable())
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Read on write-only file", "MemoryFile.read()");

		return _read(buf, size);
	}

	//------------------------------------
	File& MemoryFile::_read(void* buf, file_size_t size) {
		file_size_t resSize = size;
		file_size_t actp = mFilePos;

		// actual position in the result buffer
		char* ap = (char *)buf;

		std::pair<size_t, int> pgp = decomposePos(actp);

		while (resSize > 0) { // read from one page in one cycle pass
			file_size_t read = 0;

			char *page = mPages.at(pgp.first);

			uint toEnd;

			if (pgp.first >= mPages.size()) { // last page. or error
				toEnd = (mSize % MEMORY_FILE_BUF_LEN) - pgp.second;

				if (toEnd == 0 || pgp.first >= mPages.size()) {
					// if this occurs, I've read the bytes I could
					// I also should set the important member vars prior to exception throw
					mEof = true;
					mFilePos = mSize;

					OPDE_FILEEXCEPT(FILE_READ_ERROR, "Read past end of file", "MemoryFile.read()");
				}
			} else {
				toEnd = MEMORY_FILE_BUF_LEN - pgp.second;
			}



			read = (resSize > toEnd) ? toEnd : resSize;

			memcpy((void *)ap, (void *)(&page[pgp.second]), read);

			// increment the pointers and counts

			pgp.first++; // Next page
			pgp.second = 0;

			ap += read;
			actp += read;
			resSize -= read;
			mFilePos = actp;
		}

		if (mFilePos == mSize)
			mEof = true;

		return *this;

	}

	//------------------------------------
	File& MemoryFile::write(const void* buf, file_size_t size) {
		if (!isWriteable())
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Write on read-only file", "MemoryFile.write()");

		return _write(buf, size);
	}

	//------------------------------------
	File& MemoryFile::_write(const void* buf, file_size_t size) {
		file_size_t resSize = size;
		file_size_t actp = mFilePos;

		// actual position in the source buffer
		std::pair<size_t, int> pgp = decomposePos(actp);

		while (resSize > 0) {
			file_size_t written = 0;

			if (pgp.first >= mPages.size()) { // I've finished the already allocated pages
				char *nbuf = new char[MEMORY_FILE_BUF_LEN];

				if (nbuf == NULL) // this is quite fatal. something like out of disk space
					OPDE_FILEEXCEPT(FILE_WRITE_ERROR, "Out of memory when allocating a new page", "MemoryFile.write()");

				mPages.push_back(nbuf);
			}

			char *page = mPages.at(pgp.first);

			uint toEnd;

			toEnd = MEMORY_FILE_BUF_LEN - pgp.second;

			written = (resSize > toEnd) ? toEnd : resSize;

			memcpy((void *)(&page[pgp.second]), buf, written);


			// increment the pointers and counts
			// next page
			pgp.first++;
			pgp.second = 0;

			buf = (char *)(buf) + written;
			actp += written;
			resSize -= written;
			mFilePos = actp;

			// file size validator
			if (mSize < actp) { // past end write
				mSize = actp;
				mEof = true;
			}
		}

		return *this;
	}

	//------------------------------------
	bool MemoryFile::eof() const {
		return mEof;
	}

	//------------------------------------
	void MemoryFile::initFromFile(File& src, file_size_t size) {
		if (mSize != 0)
			OPDE_FILEEXCEPT(FILE_WRITE_ERROR, "Can't do init on already used file", "MemoryFile.initFromFile()");

		while (size > 0) {
			char temp[MEMORY_FILE_BUF_LEN];

			file_size_t sz = (size > MEMORY_FILE_BUF_LEN) ? MEMORY_FILE_BUF_LEN : size;

			src.read(temp, sz);
			_write(temp, sz);

			size -= sz;
		}

		// go to the beginning of the file
		seek(0);
	}

	/*----------------------------------------------------*/
	/*-------------------- FilePart ----------------------*/
	/*----------------------------------------------------*/

	//------------------------------------
	FilePart::FilePart(const std::string& name, AccessMode accm, FilePtr& src, file_pos_t pos, file_size_t size)
			: File(name, accm), mFilePos(0) {

		// only allow write if the underlying supports
		if ((accm & FILE_W) && !src->isWriteable())
			OPDE_FILEEXCEPT(FILE_OPEN_ERROR, string("AccessMode mismatch opening") + name
					, "FilePart::FilePart()");

		mSrcFile = src;

		mOffPos = pos;
		mSize = size;
		mEof = (mSize <= mFilePos); // Eof if the size is 0
	}

	//------------------------------------
	FilePart::~FilePart() {
	}

	//------------------------------------
	const file_size_t FilePart::size() {
		return mSize;
	}


	//------------------------------------
	void FilePart::seek(file_offset_t pos, SeekMode mode) {
		// just set the internal ptr
		file_pos_t npos = mFilePos; // new position

		switch (mode) {
			case FSEEK_BEG :
				npos = pos;
				break;
			case FSEEK_END :
				npos = mSize - pos;
				break;
			case FSEEK_CUR :
				npos += pos;
				break;

			default:  // should not happen
				OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Unknown seek position modifier", "MemoryFile.seek()");
		}

		if (npos > mSize)
			OPDE_FILEEXCEPT(FILE_OP_FAILED, "Resulting position not within the file size", "MemoryFile::seek()");

		mFilePos = npos;

		mEof = (mFilePos >= mSize);
	}


	//------------------------------------
	void FilePart::seek(file_pos_t pos) {
		seek(pos, FSEEK_BEG);
	}

	//------------------------------------
	const file_pos_t FilePart::tell() {
		return mFilePos;
	}

	//------------------------------------
	File& FilePart::read(void* buf, file_size_t size) {
		backupPos();

		// only allow reads that do not overlap file's end. Otherwise, throw an exception
		if (mFilePos + size > mSize)
			OPDE_FILEEXCEPT(FILE_READ_ERROR, "Read past end of file", "FilePart.read()");

		mSrcFile->seek(mOffPos + mFilePos);
		mSrcFile->read(buf, size);

		mFilePos += size;
		mEof = (mFilePos >= mSize);

		restorePos();

		return *this;
	}

	//------------------------------------
	File& FilePart::write(const void* buf, file_size_t size) {
		backupPos();

		if (!isWriteable())
			OPDE_FILEEXCEPT(FILE_WRITE_ERROR, "Write not enabled on this FilePart", "FilePart::write");

		if (mFilePos + size > mSize)
			OPDE_FILEEXCEPT(FILE_WRITE_ERROR, "Write would reach past FilePart's end", "FilePart::write");

		mSrcFile->seek(mOffPos + mFilePos);
		mSrcFile->write(buf, size);

		mFilePos += size;
		mEof = (mFilePos >= mSize);

		restorePos();

		return *this;
	}

	//------------------------------------
	bool FilePart::eof() const {
		return mEof;
	}

	//------------------------------------
	void FilePart::backupPos() {
		mPrevPos = mSrcFile->tell();
	}

	//------------------------------------
	void FilePart::restorePos() {
		mSrcFile->seek(mPrevPos);
	}
}
