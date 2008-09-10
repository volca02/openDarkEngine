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
 *		$Id$
 *
 *****************************************************************************/
 
#ifndef __DTYPESCRIPTLOADER_H
#define __DTYPESCRIPTLOADER_H

#include "config.h"

#include <OgreDataStream.h>
#include <OgreScriptLoader.h>
#include <OgreStringVector.h>

#include "DTypeScriptCompiler.h"

namespace Opde {

	/** Loader for .dtype files. Registers itself as a ScriptLoader in the ResourceGroupManager. Automatically parses all encountered *.dtype files. */
	class OPDELIB_EXPORT DTypeScriptLoader : public Ogre::ScriptLoader {
		public:
			/** Constructor. Registers itself as a ScriptLoader to ResourceGroupManager */
			DTypeScriptLoader();
			
			/** Destructor. Unegisters itself from ResourceGroupManager */
			virtual	~DTypeScriptLoader();
			
			/** Patterns that are this kind of script. 
			* @return "*.dtype" */
			const Ogre::StringVector& getScriptPatterns(void) const;
			
			/** Called when dtype script is found. Calls DTypeScriptCompiler::parseScript */
 			void parseScript(Ogre::DataStreamPtr &stream, const Ogre::String &groupName);
			
			/** Loading order. 
			@return 1 and this should be less than all scripts requiring the binary definitions */
			Ogre::Real getLoadingOrder(void) const;
			
 		protected:
			/** Used compiler */
			DTypeScriptCompiler* mDTypeCompiler;
			
			/** The patterns that are dtype */
			Ogre::StringVector mScriptPatterns;
	};
}

#endif

