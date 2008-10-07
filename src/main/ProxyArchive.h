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
 
#ifndef __PROXYARCHIVE_H
#define __PROXYARCHIVE_H

#include "config.h"
#include "OgreArchive.h"
 
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
			ProxyArchive(const String& name, const String& archType);
			
			/// destructor
			virtual ~ProxyArchive(void);
			
			/// performs the archive load. Scans the archive for filenames, builds the reverse transform table
			virtual void load(void);
			
			/// Unloads the archive, clears the transform map
			virtual void unload(void);
			
			/// opens a resource stream, unmapping the name first
			virtual DataStreamPtr open(const String& filename) const;
			
			/// lists the contents of the archive. transformed.
			virtual StringVectorPtr list(bool recursive = true, bool dirs = false);
			
			/// lists the contents of the archive. transformed, in the FileInfo structures.
			virtual FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false);
			
			/// performs a pattern match find on the archive files
			virtual StringVectorPtr find(const String& pattern, bool recursive = true,
            bool dirs = false) = 0;
            
            /// Searches for the given name, untransforming it first
            virtual bool exists(const String& filename);
            
            /** Searches for files that match the given pattern
            * @see find
			*/
			virtual FileInfoListPtr findFileInfo(const String& pattern, 
				bool recursive = true, bool dirs = false);
			
			/// reports case sensitiveness of this proxy archive
			virtual bool isCaseSensitive(void) const = 0;
			
		protected:
			/// performs the forward transform on a single name
			virtual String transformName(const String& name) = 0;
			
			/// performs an inverse transform on a single name, turning internal name from the external one
			virtual String untransformName(const String& name) const;
			
			/** compares if the given filename matches the pattern
			* By default, this does case insensitive filename match on the untransformed name
			*/
			virtual bool match(const String& pattern, const String& name) const;
			
			/// constructs the archive. called from constructor
			virtual void createArchive(void);
			
			/// destructs the archive. called from destructor
			virtual void destroyArchive(void);
		
			typedef std::map<std::string, std::string> NameTable;
			
			NameTable mExtToIntNames;
			
			Archive* mArchive;
	};
	
	/*class CaseLessFileSystemArchive : public ProxyArchive<FileSystemArchive> {
		protected:
			virtual std::string transformName(const std::string& name) = 0;
	}
	
	/// factory for the various types of ProxyArchive
	class ProxyArchiveFactory<class PA> {
		// TODO: finish this
	}
	*/
}

#endif
