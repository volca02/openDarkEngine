// TEMPORARILY ADDED TO OPDE AS A FIX TO OGRE 1.9 BUG
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#if OGRE_NO_ZIP_ARCHIVE == 0

#include "OgreZip.h"

#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"

#include <zzip/zzip.h>
#include <zzip/plugin.h>


namespace Ogre {

    /// Utility method to format out zzip errors
    String getZzipErrorDescription(zzip_error_t zzipError)
    {
        String errorMsg;
        switch (zzipError)
        {
        case ZZIP_NO_ERROR:
            break;
        case ZZIP_OUTOFMEM:
            errorMsg = "Out of memory.";
            break;
        case ZZIP_DIR_OPEN:
        case ZZIP_DIR_STAT:
        case ZZIP_DIR_SEEK:
        case ZZIP_DIR_READ:
            errorMsg = "Unable to read zip file.";
            break;
        case ZZIP_UNSUPP_COMPR:
            errorMsg = "Unsupported compression format.";
            break;
        case ZZIP_CORRUPTED:
            errorMsg = "Corrupted archive.";
            break;
        default:
            errorMsg = "Unknown error.";
            break;
        };

        return errorMsg;
    }
    //-----------------------------------------------------------------------
    FixedZipArchive::FixedZipArchive(const String& name, const String& archType, zzip_plugin_io_handlers* pluginIo)
        : Archive(name, archType), mZzipDir(0), mPluginIo(pluginIo)
    {
    }
    //-----------------------------------------------------------------------
    FixedZipArchive::~FixedZipArchive()
    {
        unload();
    }
    //-----------------------------------------------------------------------
    void FixedZipArchive::load()
    {
		OGRE_LOCK_AUTO_MUTEX;
        if (!mZzipDir)
        {
            zzip_error_t zzipError;
            mZzipDir = zzip_dir_open_ext_io(mName.c_str(), &zzipError, 0, mPluginIo);
            checkZzipError(zzipError, "opening archive");

            // Cache names
            ZZIP_DIRENT zzipEntry;
            while (zzip_dir_read(mZzipDir, &zzipEntry))
            {
                FileInfo info;
				info.archive = this;
                // Get basename / path
                StringUtil::splitFilename(zzipEntry.d_name, info.basename, info.path);
                info.filename = zzipEntry.d_name;
                // Get sizes
                info.compressedSize = static_cast<size_t>(zzipEntry.d_csize);
                info.uncompressedSize = static_cast<size_t>(zzipEntry.st_size);
                // folder entries
                if (info.basename.empty())
                {
                    info.filename = info.filename.substr (0, info.filename.length () - 1);
                    StringUtil::splitFilename(info.filename, info.basename, info.path);
                    // Set compressed size to -1 for folders; anyway nobody will check
                    // the compressed size of a folder, and if he does, its useless anyway
                    info.compressedSize = size_t (-1);
                }

                mFileList.push_back(info);
            }

        }
    }
    //-----------------------------------------------------------------------
    void FixedZipArchive::unload()
    {
		OGRE_LOCK_AUTO_MUTEX;
        if (mZzipDir)
        {
            zzip_dir_close(mZzipDir);
            mZzipDir = 0;
            mFileList.clear();
        }

    }
    //-----------------------------------------------------------------------
	DataStreamPtr FixedZipArchive::open(const String& filename, bool readOnly) const
    {
		// zziplib is not threadsafe
		OGRE_LOCK_AUTO_MUTEX;
        String lookUpFileName = filename;

        // Format not used here (always binary)
        ZZIP_FILE* zzipFile =
            zzip_file_open(mZzipDir, lookUpFileName.c_str(), ZZIP_ONLYZIP | ZZIP_CASELESS);
        if (!zzipFile) // Try if we find the file
        {
            const Ogre::FileInfoListPtr fileNfo = findFileInfo(lookUpFileName, true);
            if (fileNfo->size() == 1) // If there are more files with the same do not open anyone
            {
                Ogre::FileInfo info = fileNfo->at(0);
                lookUpFileName = info.path + info.basename;
                zzipFile = zzip_file_open(mZzipDir, lookUpFileName.c_str(), ZZIP_ONLYZIP | ZZIP_CASELESS); // When an error happens here we will catch it below
            }
        }

        if (!zzipFile)
		{
            int zerr = zzip_error(mZzipDir);
            String zzDesc = getZzipErrorDescription((zzip_error_t)zerr);
            LogManager::getSingleton().logMessage(
                mName + " - Unable to open file " + lookUpFileName + ", error was '" + zzDesc + "'", LML_CRITICAL);

			// return null pointer
			return DataStreamPtr();
		}

		// Get uncompressed size too
		ZZIP_STAT zstat;
		zzip_dir_stat(mZzipDir, lookUpFileName.c_str(), &zstat, ZZIP_CASEINSENSITIVE);

        // Construct & return stream
        return DataStreamPtr(OGRE_NEW ZipDataStream(lookUpFileName, zzipFile, static_cast<size_t>(zstat.st_size)));

    }
	//---------------------------------------------------------------------
	DataStreamPtr FixedZipArchive::create(const String& filename) const
	{
		OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
			"Modification of zipped archives is not supported",
			"FixedZipArchive::create");

	}
	//---------------------------------------------------------------------
	void FixedZipArchive::remove(const String& filename) const
	{
	}
    //-----------------------------------------------------------------------
    StringVectorPtr FixedZipArchive::list(bool recursive, bool dirs)
    {
		OGRE_LOCK_AUTO_MUTEX;
        StringVectorPtr ret = StringVectorPtr(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);

        FileInfoList::iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || i->path.empty()))
                ret->push_back(i->filename);

        return ret;
    }
    //-----------------------------------------------------------------------
    FileInfoListPtr FixedZipArchive::listFileInfo(bool recursive, bool dirs)
    {
		OGRE_LOCK_AUTO_MUTEX;
        FileInfoList* fil = OGRE_NEW_T(FileInfoList, MEMCATEGORY_GENERAL)();
        FileInfoList::const_iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || i->path.empty()))
                fil->push_back(*i);

        return FileInfoListPtr(fil, SPFM_DELETE_T);
    }
    //-----------------------------------------------------------------------
    StringVectorPtr FixedZipArchive::find(const String& pattern, bool recursive, bool dirs)
    {
		OGRE_LOCK_AUTO_MUTEX;
        StringVectorPtr ret = StringVectorPtr(OGRE_NEW_T(StringVector, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);
		bool wildCard = pattern.find("*") != String::npos;

        FileInfoList::iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || full_match || wildCard))
                // Check basename matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? i->filename : i->basename, pattern, false))
                    ret->push_back(i->filename);

        return ret;
    }
    //-----------------------------------------------------------------------
	FileInfoListPtr FixedZipArchive::findFileInfo(const String& pattern,
        bool recursive, bool dirs) const
    {
		OGRE_LOCK_AUTO_MUTEX;
        FileInfoListPtr ret = FileInfoListPtr(OGRE_NEW_T(FileInfoList, MEMCATEGORY_GENERAL)(), SPFM_DELETE_T);
        // If pattern contains a directory name, do a full match
        bool full_match = (pattern.find ('/') != String::npos) ||
                          (pattern.find ('\\') != String::npos);
		bool wildCard = pattern.find("*") != String::npos;

        FileInfoList::const_iterator i, iend;
        iend = mFileList.end();
        for (i = mFileList.begin(); i != iend; ++i)
            if ((dirs == (i->compressedSize == size_t (-1))) &&
                (recursive || full_match || wildCard))
                // Check name matches pattern (zip is case insensitive)
                if (StringUtil::match(full_match ? i->filename : i->basename, pattern, false))
                    ret->push_back(*i);

        return ret;
    }
    //-----------------------------------------------------------------------
	struct FileNameCompare : public std::binary_function<FileInfo, String, bool>
	{
		bool operator()(const Ogre::FileInfo& lhs, const String& filename) const
		{
			return lhs.filename == filename;
		}
	};
	//-----------------------------------------------------------------------
	bool FixedZipArchive::exists(const String& filename)
	{
		OGRE_LOCK_AUTO_MUTEX;
		String cleanName = filename;
		if(filename.rfind("/") != String::npos)
		{
			StringVector tokens = StringUtil::split(filename, "/");
			cleanName = tokens[tokens.size() - 1];
		}

		return std::find_if (mFileList.begin(), mFileList.end(), std::bind2nd<FileNameCompare>(FileNameCompare(), cleanName)) != mFileList.end();
	}
	//---------------------------------------------------------------------
	time_t FixedZipArchive::getModifiedTime(const String& filename)
	{
		// Zziplib doesn't yet support getting the modification time of individual files
		// so just check the mod time of the zip itself
		struct stat tagStat;
		bool ret = (stat(mName.c_str(), &tagStat) == 0);

		if (ret)
		{
			return tagStat.st_mtime;
		}
		else
		{
			return 0;
		}

	}
	//-----------------------------------------------------------------------
    void FixedZipArchive::checkZzipError(int zzipError, const String& operation) const
    {
        if (zzipError != ZZIP_NO_ERROR)
        {
            String errorMsg = getZzipErrorDescription(static_cast<zzip_error_t>(zzipError));

            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                mName + " - error whilst " + operation + ": " + errorMsg,
                "FixedZipArchive::checkZzipError");
        }
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    ZipDataStream::ZipDataStream(ZZIP_FILE* zzipFile, size_t uncompressedSize)
        : mZzipFile(zzipFile)
    {
		mSize = uncompressedSize;
    }
    //-----------------------------------------------------------------------
    ZipDataStream::ZipDataStream(const String& name, ZZIP_FILE* zzipFile, size_t uncompressedSize)
        :DataStream(name), mZzipFile(zzipFile)
    {
		mSize = uncompressedSize;
    }
    //-----------------------------------------------------------------------
	ZipDataStream::~ZipDataStream()
	{
		close();
	}
    //-----------------------------------------------------------------------
    size_t ZipDataStream::read(void* buf, size_t count)
    {
		size_t was_avail = mCache.read(buf, count);
		zzip_ssize_t r = 0;
		if (was_avail < count)
		{
			r = zzip_file_read(mZzipFile, (char*)buf + was_avail, count - was_avail);
			if (r<0) {
				ZZIP_DIR *dir = zzip_dirhandle(mZzipFile);
				String msg = zzip_strerror_of(dir);
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
					mName+" - error from zziplib: "+msg,
					"ZipDataStream::read");
			}
			mCache.cacheData((char*)buf + was_avail, (size_t)r);
		}
		return was_avail + (size_t)r;
    }
	//---------------------------------------------------------------------
	size_t ZipDataStream::write(void* buf, size_t count)
	{
		// not supported
		return 0;
	}
    //-----------------------------------------------------------------------
    void ZipDataStream::skip(long count)
    {
        long was_avail = static_cast<long>(mCache.avail());
		if (count > 0)
		{
			if (!mCache.ff(count))
				zzip_seek(mZzipFile, static_cast<zzip_off_t>(count - was_avail), SEEK_CUR);
		}
		else if (count < 0)
		{
			if (!mCache.rewind((size_t)(-count)))
				zzip_seek(mZzipFile, static_cast<zzip_off_t>(count + was_avail), SEEK_CUR);
		}
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::seek( size_t pos )
    {
		zzip_off_t newPos = static_cast<zzip_off_t>(pos);
		zzip_off_t prevPos = static_cast<zzip_off_t>(tell());
		if (prevPos < 0)
		{
			// seek set after invalid pos
			mCache.clear();
			zzip_seek(mZzipFile, newPos, SEEK_SET);
		}
		else
		{
			// everything is going all right, relative seek
			skip((long)(newPos - prevPos));
		}
    }
    //-----------------------------------------------------------------------
    size_t ZipDataStream::tell(void) const
    {
		zzip_off_t pos = zzip_tell(mZzipFile);
		if (pos<0)
			return (size_t)(-1);
		return static_cast<size_t>(pos) - mCache.avail();
    }
    //-----------------------------------------------------------------------
    bool ZipDataStream::eof(void) const
    {
        return (tell() >= mSize);
    }
    //-----------------------------------------------------------------------
    void ZipDataStream::close(void)
    {
		if (mZzipFile != 0)
		{
			zzip_file_close(mZzipFile);
			mZzipFile = 0;
		}
		mCache.clear();
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //  FixedZipArchiveFactory
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    const String& FixedZipArchiveFactory::getType(void) const
    {
        static String name = "FZip";
        return name;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //  EmbeddedFixedZipArchiveFactory
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    /// a struct to hold embedded file data
    struct EmbeddedFileData
    {
        const uint8 * fileData;
        zzip_size_t fileSize;
        zzip_size_t curPos;
        bool isFileOpened;
        EmbeddedFixedZipArchiveFactory::DecryptEmbeddedZipFileFunc decryptFunc;
    };
    //-----------------------------------------------------------------------
    /// A type for a map between the file names to file index
    typedef map<String, int>::type FileNameToIndexMap;
    typedef FileNameToIndexMap::iterator FileNameToIndexMapIter;
    /// A type to store the embedded files data
    typedef vector<EmbeddedFileData>::type EmbbedFileDataList;
    /// A static map between the file names to file index
    FileNameToIndexMap * EmbeddedFixedZipArchiveFactory_mFileNameToIndexMap;
    /// A static list to store the embedded files data
    EmbbedFileDataList * EmbeddedFixedZipArchiveFactory_mEmbbedFileDataList;
    /// A static pointer to file io alternative implementation for the embedded files
    zzip_plugin_io_handlers* EmbeddedFixedZipArchiveFactory::mPluginIo = NULL;
    _zzip_plugin_io sEmbeddedFixedZipArchiveFactory_PluginIo;
    #define EMBED_IO_BAD_FILE_HANDLE (-1)
    #define EMBED_IO_SUCCESS (0)
    //-----------------------------------------------------------------------
    /// functions for embedded zzip_plugin_io_handlers implementation
    /// The functions are here and not as static members because they
    /// use types that I don't want to define in the header like zzip_char_t,
    //  zzip_ssize_t and such.
    //-----------------------------------------------------------------------
    // get file date by index
    EmbeddedFileData & getEmbeddedFileDataByIndex(int fd)
    {
        return (*EmbeddedFixedZipArchiveFactory_mEmbbedFileDataList)[fd-1];
    }
    //-----------------------------------------------------------------------
    // opens the file
    int EmbeddedFixedZipArchiveFactory_open(zzip_char_t* name, int flags, ...)
    {
        String nameAsString = name;
        FileNameToIndexMapIter foundIter = EmbeddedFixedZipArchiveFactory_mFileNameToIndexMap->find(nameAsString);
        if (foundIter != EmbeddedFixedZipArchiveFactory_mFileNameToIndexMap->end())
        {
            int fd = foundIter->second;
            EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
            if(curEmbeddedFileData.isFileOpened)
            {
               // file is opened - return an error handle
               return EMBED_IO_BAD_FILE_HANDLE;
            }

            curEmbeddedFileData.isFileOpened = true;
            return fd;
        }
        else
        {
           // not found - return an error handle
           return EMBED_IO_BAD_FILE_HANDLE;
        }
    }
    //-----------------------------------------------------------------------
    // Closes a file.
    // Return Value - On success, close returns 0.
    int EmbeddedFixedZipArchiveFactory_close(int fd)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error
            return -1;
        }

        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);

        if(curEmbeddedFileData.isFileOpened == false)
        {
           // file is opened - return an error handle
           return -1;
        }
        else
        {
            // success
            curEmbeddedFileData.isFileOpened = false;
            curEmbeddedFileData.curPos = 0;
            return 0;
        }

    }

    //-----------------------------------------------------------------------
    // reads data from the file
    zzip_ssize_t EmbeddedFixedZipArchiveFactory_read(int fd, void* buf, zzip_size_t len)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error size - negative
            return -1;
        }
        // get the current buffer in file;
        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
        const uint8 * curFileData = curEmbeddedFileData.fileData;
        if (len + curEmbeddedFileData.curPos > curEmbeddedFileData.fileSize)
        {
            len = curEmbeddedFileData.fileSize - curEmbeddedFileData.curPos;
        }
        curFileData += curEmbeddedFileData.curPos;

        // copy to out buffer
        memcpy(buf, curFileData, len);

        if( curEmbeddedFileData.decryptFunc != NULL )
        {
            if (!curEmbeddedFileData.decryptFunc(curEmbeddedFileData.curPos, buf, len))
            {
                // decrypt failed - return an error size - negative
                return -1;
            }
        }

        // move the cursor to the new pos
        curEmbeddedFileData.curPos += len;

        return len;
    }
    //-----------------------------------------------------------------------
    // Moves file pointer.
    zzip_off_t EmbeddedFixedZipArchiveFactory_seeks(int fd, zzip_off_t offset, int whence)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error - nonzero value.
            return -1;
        }

        zzip_size_t newPos = -1;
        // get the current buffer in file;
        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
        switch(whence)
        {
            case SEEK_CUR:
                newPos = (zzip_size_t)(curEmbeddedFileData.curPos + offset);
                break;
            case SEEK_END:
                newPos = (zzip_size_t)(curEmbeddedFileData.fileSize - offset);
                break;
            case SEEK_SET:
                newPos = (zzip_size_t)offset;
                break;
            default:
                // bad whence - return an error - nonzero value.
                return -1;
                break;
        };
        if (newPos >= curEmbeddedFileData.fileSize)
        {
            // bad whence - return an error - nonzero value.
            return -1;
        }

        curEmbeddedFileData.curPos = newPos;
        return newPos;
    }
    //-----------------------------------------------------------------------
    // returns the file size
    zzip_off_t EmbeddedFixedZipArchiveFactory_filesize(int fd)
    {
        if (fd == EMBED_IO_BAD_FILE_HANDLE)
        {
            // bad index - return an error - nonzero value.
            return -1;
        }
                // get the current buffer in file;
        EmbeddedFileData & curEmbeddedFileData = getEmbeddedFileDataByIndex(fd);
        return curEmbeddedFileData.fileSize;
    }
    //-----------------------------------------------------------------------
    // writes data to the file
    zzip_ssize_t EmbeddedFixedZipArchiveFactory_write(int fd, _zzip_const void* buf, zzip_size_t len)
    {
        // the files in this case are read only - return an error  - nonzero value.
        return -1;
    }
    //-----------------------------------------------------------------------
    EmbeddedFixedZipArchiveFactory::EmbeddedFixedZipArchiveFactory()
    {
        // init static member
        if (mPluginIo == NULL)
        {
            mPluginIo = &sEmbeddedFixedZipArchiveFactory_PluginIo;
            mPluginIo->fd.open = EmbeddedFixedZipArchiveFactory_open;
            mPluginIo->fd.close = EmbeddedFixedZipArchiveFactory_close;
            mPluginIo->fd.read = EmbeddedFixedZipArchiveFactory_read;
            mPluginIo->fd.seeks = EmbeddedFixedZipArchiveFactory_seeks;
            mPluginIo->fd.filesize = EmbeddedFixedZipArchiveFactory_filesize;
            mPluginIo->fd.write = EmbeddedFixedZipArchiveFactory_write;
            mPluginIo->fd.sys = 1;
            mPluginIo->fd.type = 1;
        }
    }
    //-----------------------------------------------------------------------
    EmbeddedFixedZipArchiveFactory::~EmbeddedFixedZipArchiveFactory()
    {
    }
    //-----------------------------------------------------------------------
    const String& EmbeddedFixedZipArchiveFactory::getType(void) const
    {
        static String name = "EmbeddedFZip";
        return name;
    }
    //-----------------------------------------------------------------------
    void EmbeddedFixedZipArchiveFactory::addEmbbeddedFile(const String& name, const uint8 * fileData,
                                        size_t fileSize, DecryptEmbeddedZipFileFunc decryptFunc)
    {
        static bool needToInit = true;
        if(needToInit)
        {
            needToInit = false;

            // we can't be sure when global variables get initialized
            // meaning it is possible our list has not been init when this
            // function is being called. The solution is to use local
            // static members in this function an init the pointers for the
            // global here. We know for use that the local static variables
            // are create in this stage.
            static FileNameToIndexMap sFileNameToIndexMap;
            static EmbbedFileDataList sEmbbedFileDataList;
            EmbeddedFixedZipArchiveFactory_mFileNameToIndexMap = &sFileNameToIndexMap;
            EmbeddedFixedZipArchiveFactory_mEmbbedFileDataList = &sEmbbedFileDataList;
        }

        EmbeddedFileData newEmbeddedFileData;
        newEmbeddedFileData.curPos = 0;
        newEmbeddedFileData.isFileOpened = false;
        newEmbeddedFileData.fileData = fileData;
        newEmbeddedFileData.fileSize = fileSize;
        newEmbeddedFileData.decryptFunc = decryptFunc;
        EmbeddedFixedZipArchiveFactory_mEmbbedFileDataList->push_back(newEmbeddedFileData);
        (*EmbeddedFixedZipArchiveFactory_mFileNameToIndexMap)[name] = static_cast<int>(EmbeddedFixedZipArchiveFactory_mEmbbedFileDataList->size());
    }
    //-----------------------------------------------------------------------
    void EmbeddedFixedZipArchiveFactory::removeEmbbeddedFile( const String& name )
    {
        EmbeddedFixedZipArchiveFactory_mFileNameToIndexMap->erase(name);
    }
}

#endif
