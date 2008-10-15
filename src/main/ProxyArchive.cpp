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

#include "ProxyArchive.h"

#include <OgreException.h>
#include <OgreString.h>

namespace Ogre {

	// -------------------------------------------------------
	ProxyArchive::ProxyArchive(const String& name, const String& archType) : 
		Archive(name, archType) {
			
		createArchive();
		assert(mArchive);
	}

	// -------------------------------------------------------
	ProxyArchive::~ProxyArchive(void) {
		destroyArchive();
	}
	
	// -------------------------------------------------------
	void ProxyArchive::load(void) {
		// load, build map
		mArchive->load();
		
		StringVectorPtr lst = mArchive->list(true, false);
		
		StringVector::iterator it = lst->begin();
		
		while (it != lst->end()) {
			const std::string& fn = *it++;
			std::string tn = transformName(fn);
			// insert into the map
			std::pair<NameTable::iterator, bool> result = 
				mExtToIntNames.insert(std::make_pair(tn, fn));
			
			// see if the result is ok, except if not
			if (!result.second)
				OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
					"Archive '" + mName + "' contains duplicities : " + fn,
					"ProxyArchive::load");
		}
	}
			
			
	// -------------------------------------------------------
	void ProxyArchive::unload(void) {
		mArchive->unload();
		
		mExtToIntNames.clear();
	}
	
	// -------------------------------------------------------
	FileInfoListPtr ProxyArchive::findFileInfo(const String& pattern, bool recursive , bool dirs) {
		/// have to list all infos, filter those which fit
		FileInfoListPtr lst = listFileInfo(recursive, dirs);
		
		FileInfoListPtr res(new FileInfoList());
		
		/// now iterate, the list, match using the name, return
		FileInfoList::iterator it = lst->begin();
		
		while (it != lst->end()) {
			const FileInfo& fi = *it++;
			// match?
			if (match(pattern, fi.filename)) {
				res->push_back(fi);
			}
		}
		
		return res;
	}

	// -------------------------------------------------------
	bool ProxyArchive::exists(const String& filename) {
		// look in the map
		NameTable::iterator it = mExtToIntNames.find(filename);
		
		if (it != mExtToIntNames.end()) {
			return mArchive->exists(it->second);
		}
		
		return false;
	}

	// -------------------------------------------------------
	StringVectorPtr ProxyArchive::find(const String& pattern, bool recursive , bool dirs) {
		/// have to list all infos, filter those which fit
		StringVectorPtr lst = list(recursive, dirs);
		
		StringVectorPtr res(new StringVector());
		
		/// now iterate, the list, match using the name, return
		StringVector::iterator it = lst->begin();
		
		while (it != lst->end()) {
			const String& fn = *it++;
			// match?
			if (match(pattern, fn)) {
				res->push_back(fn);
			}
		}
		
		return res;
	}

	// -------------------------------------------------------
	FileInfoListPtr ProxyArchive::listFileInfo(bool recursive , bool dirs) {
		FileInfoListPtr list = mArchive->listFileInfo(recursive, dirs);
		
		FileInfoListPtr res(new FileInfoList());
		
		/// now iterate, the list, match using the name, return
		FileInfoList::iterator it = list->begin();
		
		while (it != list->end()) {
			const FileInfo& fi = *it++;
		
			FileInfo fin;
			
			fin.archive = this;
			fin.filename = transformName(fi.filename);
			fin.path = transformName(fi.path);
			fin.basename = transformName(fi.basename);
			
			fin.compressedSize = fi.compressedSize;
			fin.uncompressedSize = fi.uncompressedSize;
			
			res->push_back(fin);
		}
		
		return res;
	}

	// -------------------------------------------------------
	StringVectorPtr ProxyArchive::list(bool recursive , bool dirs) {
		/// have to list all infos, filter those which fit
		StringVectorPtr lst = list(recursive, dirs);
		
		StringVectorPtr res(new StringVector());
		
		/// now iterate, the list, match using the name, return
		StringVector::iterator it = lst->begin();
		
		while (it != lst->end()) {
			const String& fn = *it++;
			
			res->push_back(transformName(fn));
		}
		
		return res;
	}

	// -------------------------------------------------------
	DataStreamPtr ProxyArchive::open(const String& filename) const {
		String utfn = untransformName(filename);
		return mArchive->open(utfn);
	}

	// -------------------------------------------------------
	std::string ProxyArchive::untransformName(const String& name) const {
		NameTable::const_iterator it = mExtToIntNames.find(name);
		
		if (it != mExtToIntNames.end()) {
			return it->second;
		} else {
			OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND,
					"Archive '" + mName + "' does not contain file : " + name,
					"ProxyArchive::untransformName");
		}
	}
	
	// -------------------------------------------------------
	bool ProxyArchive::match(const String& pattern, const String& name) const {
		String unt = untransformName(name);
		StringUtil::toLowerCase(unt);
		String lpattern = pattern;
		StringUtil::toLowerCase(lpattern);
		
		// inspired by one codeproject article (but a rewrite without using the code)

		enum State {
			PM_Match = 0,
			PM_Any,
			PM_AnySeq
		};

		// we don't wanna depend on some lib that does not exist for visual C anyway
		bool match = true;
		String::iterator pi = lpattern.begin();
		String::iterator pnext = pi; // only used for AnySeq
		String::iterator si = unt.begin();
		int state = 0;

		while (match && pi != lpattern.end()) {
			// if we're at the end of the source string
			if (si == unt.end()) {
				// match is found if pi got to end as well
				match = pi == lpattern.end();
				break;
			}

			// see what we're comparing to
			state = PM_Match;

			if (*pi == '*') {
				state = PM_AnySeq;
				pnext = pi;

				// eat all the consecutive stars
				while (*pnext == '*')
						++pnext;
			} else if (*pi == '?') {
				state = PM_Any;
			}

			switch (state) {
				case PM_Match: 
					match = *pi == *si;
				case PM_Any:
					++pi; ++si;
					break;
				case PM_AnySeq:
					// matched any character
					++si;
					
					// found the end of pattern? If so,
					// then the previous comparisons decide
					if (pnext == lpattern.end()) {
							match = true;
							break;
					}

					// end for string, but some pattern following?
					if (pnext != lpattern.end() &&
						si == unt.end()) {
							match = false;
							break;
					}

					// or found a match after the star?
					if (*si == *pnext) ++pi;
					break;
			}
		}

		return match;
	}
	
	// -------------------------------------------------------
	void ProxyArchive::createArchive(void) {
		mArchive = NULL;
	}
			
			
	// -------------------------------------------------------
	void ProxyArchive::destroyArchive(void) {
		// nothing
	}
	
}
