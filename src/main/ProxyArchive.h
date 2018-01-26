/******************************************************************************
 *
 *	  This file is part of openDarkEngine project
 *	  Copyright (C) 2005-2006 openDarkEngine team
 *
 *	  This program is free software; you can redistribute it and/or modify
 *	  it under the terms of the GNU General Public License as published by
 *	  the Free Software Foundation; either version 2 of the License, or
 *	  (at your option) any later version.
 *
 *	  This program is distributed in the hope that it will be useful,
 *	  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	  GNU General Public License for more details.
 *
 *	  You should have received a copy of the GNU General Public License
 *	  along with this program; if not, write to the Free Software
 *	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
 *
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __PROXYARCHIVE_H
#define __PROXYARCHIVE_H

#include "config.h"

#include <OgreArchive.h>
#include <OgreArchiveFactory.h>

namespace Ogre {

	/** Proxy archive, which performs name transforms and delegates to underlying archive instance.
	* There are couple different possible usages for this archive class:
	* 1. A case insensitive filesystem archive on *nix systems
	* 2. A ZIP file handler which includes the FNAME.ZIP as FNAME/ in all names of the archive
	* 3. Other
	*
	*
	* In Opde, this class is used to mask the fact about the resource's source (crf file or not), and
	* make ogre's file system archive case insensitive on *nix systems.
	*
	* @todo The final piece of puzzle will be a class that autoloads crf files as resources \
	*	for the predefined group, and a resource listener that allows duplicate resource names in some way
	*/
	class OPDELIB_EXPORT ProxyArchive : public Archive {
		public:
			/// constructor
			ProxyArchive(const String& name, const String& archType, bool readOnly);

			/// destructor
			virtual ~ProxyArchive(void);

			/// performs the archive load. Scans the archive for filenames, builds the reverse transform table
			virtual void load(void);

			/// Unloads the archive, clears the transform map
			virtual void unload(void);

			/// opens a resource stream, unmapping the name first
			virtual DataStreamPtr open(const String& filename, bool readOnly) const;

			/// lists the contents of the archive. transformed.
			virtual StringVectorPtr list(bool recursive = true, bool dirs = false) const;

			/// lists the contents of the archive. transformed, in the FileInfo structures.
			virtual FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false) const;

			/// performs a pattern match find on the archive files
			virtual StringVectorPtr find(const String& pattern, bool recursive = true,
			bool dirs = false) const;

			/// Searches for the given name, untransforming it first
			virtual bool exists(const String& filename) const;

			/** Searches for files that match the given pattern
			* @see find
			*/
			virtual FileInfoListPtr findFileInfo(const String& pattern,
				bool recursive = true, bool dirs = false) const;

			/// reports case sensitiveness of this proxy archive
			virtual bool isCaseSensitive(void) const = 0;

			time_t getModifiedTime(const String& filename) const {return 0;};

		protected:
			/// This implements a const version of listFileInfo, as ogre is inconsistent in the const modifier usage...
			FileInfoListPtr listFileInfoImpl(bool recursive = true, bool dirs = false) const;

			/// performs the forward transform on a single name
			virtual String transformName(const String& name) const = 0;

			/// performs an inverse transform on a single name, turning internal name from the external one
			virtual bool untransformName(const String& name, String& unt) const;

			/** compares if the given filename matches the pattern
			* By default, this does case insensitive filename match on the untransformed name
			*/
			virtual bool match(const String& pattern, const String& name) const;

			typedef std::map<std::string, std::string> NameTable;

			NameTable mExtToIntNames;

			Archive* mArchive;
	};

	/// Lowercase transforming file system archive
	class CaseLessFileSystemArchive : public ProxyArchive {
		public:
			CaseLessFileSystemArchive(const String& name, const String& archType, bool readOnly);
			~CaseLessFileSystemArchive(void);

			bool isCaseSensitive(void) const;

		protected:
			String transformName(const std::string& name) const;

	};

	/// Zip archive wrapper that prefixes the file names with the name without extesion (fam.crf -> fam/*)
	class CRFArchive : public ProxyArchive {
		public:
			CRFArchive(const String& name, const String& archType, bool readOnly);
			~CRFArchive(void);

			bool isCaseSensitive(void) const;

		protected:
			String transformName(const std::string& name) const;

			String mFilePart;
	};

	// Factories, so we can actually use these

	class OPDELIB_EXPORT CaseLessFileSystemArchiveFactory : public ArchiveFactory { // what a title!
		public:
			virtual ~CaseLessFileSystemArchiveFactory() {}

			const String& getType(void) const;

			Archive* createInstance(const String& name, bool readOnly) {
				return new CaseLessFileSystemArchive(name, "Dir", readOnly);
			}

			void destroyInstance( Archive* arch) { delete arch; }
	};

	class CrfArchiveFactory : public ArchiveFactory {
		public:
			virtual ~CrfArchiveFactory() {}

			const String& getType(void) const;

			Archive* createInstance(const String& name, bool readOnly) {
				return new CRFArchive(name, "Crf", readOnly);
			}

			void destroyInstance( Archive* arch) { delete arch; }
	};

}

#endif
