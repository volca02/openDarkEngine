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
 *
 *****************************************************************************/


#ifndef __FILE_H
#define __FILE_H

#include "config.h"

#include "OpdeException.h"
#include "OgreDataStream.h"
#include "RefCounted.h"
#include "SharedPtr.h"
#include "integers.h"
#include <fstream>

// Maximal memory file buffer size. Reading/Writing structures bigger than this will split those to fit into the buffers
#define MEMORY_FILE_BUF_LEN 64000

namespace Opde {

	/** File error reasons. Will probably grow as needed */
	typedef enum {FILE_READ_ERROR, FILE_WRITE_ERROR, FILE_OPEN_ERROR, FILE_OP_FAILED, FILE_OTHER_ERROR, FILE_UNIMPL} FileError;


	/** File size type */
	typedef std::size_t file_size_t;

	/** File seek position type - relative */
	typedef std::streamoff file_offset_t;

	/** File seek position type - absolute */
	typedef std::streampos file_pos_t;

	/** File access exception. */
	class OPDELIB_EXPORT FileException : public Opde::BasicException {
		protected:
			FileError error;
		public:
			FileException(const FileError& err, const std::string& desc,
					const std::string& src, char* file = NULL, long line = -1) : BasicException(desc,src,file,line), error(err) {
				details = std::string("FileException: " + details);
			}

			/** Helping status getter. Returns why the exception was thrown in a machine readable form */
			FileError getFileError() {
				return error;
			}
	};

	/** A usage - simplifying macro for FileException */
	#define OPDE_FILEEXCEPT(err, desc, src) throw( Opde::FileException(err, desc, src, __FILE__, __LINE__) )

	/** Abstract file access class. Provides data manipulation methods.
	* The class is exception based, that means, any error happening while reading/writing will raise an exception.
	* @todo Line read/write functionality might be useful
	*/
	class OPDELIB_EXPORT File {
		public:
			/** File open mode. Read/Write/Read+Write access */
			typedef enum {
				/** Read-only file access mode */
				FILE_R = 1,
				/** Write-only file access mode */
				FILE_W = 2,
				/** Read/write file access mode */
				FILE_RW = 3
			} AccessMode;

			/** Seek mode. Either seek from begining, from end, or actual position */
			typedef enum {
				/** Seek from file's beginning */
				FSEEK_BEG,
				/** Seek from file's end */
				FSEEK_END,
				/** Seek from file's current position */
				FSEEK_CUR
			} SeekMode;

			/** Constructor. */
			File(const std::string& name, AccessMode mode);

			/** Destructor */
			virtual ~File();

			/** Get the (current) file size in bytes
			* @return File size in bytes
			* @note This function will likely fail in some implementation, throwing FileException */
			virtual const file_size_t size() = 0;

			/** Seek into the file, offset style */
			virtual void seek(file_offset_t pos, SeekMode mode) = 0;

			/** Seek into the file, absolute position style */
			virtual void seek(file_pos_t pos) = 0;

			/** Tell the actual position
			* @return the current position in the file counted from start of the file */
			virtual const file_pos_t tell() = 0;

			/** Read unstructured data into the buffer provided
			* @return self reference (for chain reading fh.read(a,b).read(c,d); ) */
			virtual File& read(void* buf, file_size_t size) = 0;

			/** Write unstructured data into the file, starting on actual position */
			virtual File& write(const void* buf, file_size_t size) = 0;

			/** End of file checker */
			virtual bool eof() const = 0;

			/** Writes all the contents of the File into another file
			* @note Leaves the writer file position intact */
			virtual void writeToFile(File& dest);

			/** returns true if the file can be read from */
			inline bool isReadable() { return (mAccessMode | FILE_R) != 0; };

			/** returns true if the file can be written to */
			inline bool isWriteable() { return (mAccessMode | FILE_W) != 0; };

			/** Returns the access mode of the file */
			inline AccessMode getAccessMode() { return mAccessMode; };

			/** Returns the file name of this file */
			inline std::string& getName() { return mFileName; };

			/** Reads an array of elementary types(ints/floats/bools), coded little-endian
			* swapping to big-endian if on such platform.
			* @param buf The destination buffer
			* @param size the element size
			* @param count the element count */
			File& readElem(void* buf, file_size_t size, uint count = 1);

			/** Writes an array of elementary types(ints/floats/bools), little-endian formated
			* swapping from big-endian if on such platform.
			* @param buf The destination buffer
			* @param size the element size
			* @param count the element count */
			File& writeElem(const void* buf, file_size_t size, uint count = 1);

			/** Reads structured data from the file
			The format string determines how to load the data. The format is specified as this
			@li c Character (one byte). No byte swapping is done, direct read
			@li w Word (2 byte structure). Swaps bytes as expected on big-endian machines
			@li i (or f) Integer(float) - 32bit structure. Swaps
			@li q Quad - 64bit structure - Swapped
			@note The format byte can be prefixed with numbers [0-9], telling the reader to read N those structures
			@warning The struct that is read to has to be packed (surrounded with #pragma(pack,1), #pragma(pop)) as the code expects the fields to be next to each other in memmory
			@param buf The buffer to read to
			@param format The format to use
			@param ExpectedLen The numbers of bytes expected to be read/written
			@param count The count of the structures to read (use in case of struct array)
			*/
			File& readStruct(void* buf, const char* format, uint32_t ExpectedLen, uint count = 1);

			/** Writes a structure to the file. @see File::readStruct
			*/
			File& writeStruct(const void* buf, const char* format, uint32_t ExpectedLen, uint count = 1);

			/** Reads one line from a file (treating it as a text file). Strips the line ending in the process
			 */
			std::string getLine();

		protected:
			/** swaps the endianness of the given buffer
			* @param ptr the buffer to swap endianness on
			* @param size the element size
			* @param count the count of swapped elements */
			static void swapEndian(void* ptr, file_size_t size, uint count);

			std::string mFileName;
			AccessMode mAccessMode;
	};

	/** Shared file pointer */
	typedef shared_ptr<File> FilePtr;

	/** File class implementation using std::fstream class as a base */
	class OPDELIB_EXPORT StdFile : public File {
		protected:
			/** input/output stream, valid for output access */
			std::fstream mStream;
		public:
			/** Constructor. Initializes the write access to the specified file
			* @throw Opde::FileException if the mode is not FILE_WRITE, or file cannot be opened for writing
			*/
			StdFile(const std::string& name, AccessMode mode);

			/** Destructor. Closes the file stream */
			virtual ~StdFile();

			/** @copydoc File::size() */
			virtual const file_size_t size();

			/** @copydoc File::seek(file_offset_t,SeekMode) */
			virtual void seek(file_offset_t pos, SeekMode mode);

			/** @copydoc File::seek(file_pos_t) */
			virtual void seek(file_pos_t pos);

			/** @copydoc File::tell() */
			virtual const file_pos_t tell();

			/** @copydoc File::read() */
			virtual File& read(void* buf, file_size_t size);

			/** @copydoc File::write() */
			virtual File& write(const void* buf, file_size_t size);

			/** @copydoc File::eof() */
			virtual bool eof() const;

			/** Direct access to the underlying ostream.
			* @return std::fstream instance for the opened file
			*/
			std::fstream& getStream();
	};


	/** Read only File implementation using Ogre's DataStream.
	*/
	class OPDELIB_EXPORT OgreFile : public File {
		private:
			Ogre::DataStreamPtr mStream;
		public:
			/** Constructor. Takes ogre's DataStream as source. Read only. */
			OgreFile(const Ogre::DataStreamPtr& stream);

			/** @copydoc File::size() */
			virtual const file_size_t size();

			/** @copydoc File::seek(file_offset_t,SeekMode) */
			virtual void seek(file_offset_t pos, SeekMode mode);

			/** @copydoc File::seek(file_pos_t) */
			virtual void seek(file_pos_t pos);

			/** @copydoc File::tell() */
			virtual const file_pos_t tell();

			/** @copydoc File::read() */
			virtual File& read(void* buf, file_size_t size);

			/** @copydoc File::write() */
			virtual File& write(const void* buf, file_size_t size);

			/** @copydoc File::eof() */
			virtual bool eof() const;
	};

	/** Memory file. Virtual file existing in the memory. Speciality of this class is an ability to write/read the data to/from another file instance.
	* Internally, the data are organized in buffers of maximal length of MEMORY_FILE_BUF_LEN macro. */
	class OPDELIB_EXPORT MemoryFile : public File {
		protected:
			/** File page vector (one Page contains data with length up to MEMORY_FILE_BUF_LEN) */
			typedef std::vector< char* > FilePages;

			/** file pages
			* File pages holding the file data */
			FilePages mPages;

			/** Total file size */
			file_size_t	mSize;

			/** Absolute file position */
			file_size_t mFilePos;

			/** EOF indicator */
			bool mEof;

			/** Decompose the position to Buffer-number, relative buffer position
			* @note Does validate against the file size
			* @return std::pair<size_t, int> First is page index, second is relative position within page
			*/
			std::pair<size_t, int> decomposePos(file_pos_t pos);

			/** Internal write. does not do read-only checks. */
			virtual File& _write(const void* buf, file_size_t size);

			/** Internal read. Works even on write-only files. */
			virtual File& _read(void* buf, file_size_t size);
		public:
			/** @copydoc File::File() */
			MemoryFile(const std::string& name, AccessMode mode);

			/** Destructor. Releases all the allocated pages */
			~MemoryFile();

			/** @copydoc File::size() */
			virtual const file_size_t size();

			/** @copydoc File::seek(file_offset_t,SeekMode) */
			virtual void seek(file_offset_t pos, SeekMode mode);

			/** @copydoc File::seek(file_pos_t) */
			virtual void seek(file_pos_t pos);

			/** @copydoc File::tell() */
			virtual const file_pos_t tell();

			/** @copydoc File::read() */
			virtual File& read(void* buf, file_size_t size);

			/** @copydoc File::write() */
			virtual File& write(const void* buf, file_size_t size);

			/** @copydoc File::eof() */
			virtual bool eof() const;

			/** Initialize this file with the contents of another file
			* This method will read the specified quantum of bytes from the specified stream.
			* @note This operation will only work on a empty MemoryFile instance
			* @note This method will write data into the MemoryFile's buffers even if the AccessMode is read only
			* @note Will seek(0) on self after success
			* @note the src file will have the \e size bytes read from
			*/
			virtual void initFromFile(File& src, file_size_t size);
	};

	/** A wrapping class that publishes a part of a file as if it was a file on it's own.
	* @note Care is taken about the underlying file. Backup/Restore of the previous position is done. This enables multiple simultaneous usage (not threaded though) */
	class OPDELIB_EXPORT FilePart : public File {
		public:
			/** Constructor - Takes a file instance, an absolute position, and length */
			FilePart(const std::string& name, AccessMode accm, FilePtr& src, file_pos_t pos, file_size_t size);

			/** destructor */
			~FilePart();

			/** @copydoc File::size() */
			virtual const file_size_t size();

			/** @copydoc File::seek(file_offset_t,SeekMode) */
			virtual void seek(file_offset_t pos, SeekMode mode);

			/** @copydoc File::seek(file_pos_t) */
			virtual void seek(file_pos_t pos);

			/** @copydoc File::tell() */
			virtual const file_pos_t tell();

			/** @copydoc File::read() */
			virtual File& read(void* buf, file_size_t size);

			/** @copydoc File::write() */
			virtual File& write(const void* buf, file_size_t size);

			/** @copydoc File::eof() */
			virtual bool eof() const;
		private:
			/** backup previous position into the */
			void backupPos();

			/** restore the previous stored position */
			void restorePos();


			/** Source (underlying) file */
			FilePtr	mSrcFile;

			/** Offset position - offset from the start of the underlying to this part's */
			file_size_t mOffPos;

			/** FilePart's size */
			file_size_t mSize;

			/** Absolute file position. This instance's one */
			file_size_t mFilePos;

			/** Absolute file position - previous (backup/restore) */
			file_size_t mPrevPos;

			/** eof indicator */
			bool mEof;
	};
}

#endif
