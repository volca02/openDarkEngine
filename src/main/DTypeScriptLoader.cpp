/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/
 
#include "DTypeScriptLoader.h"
#include <OgreDataStream.h>
#include <OgreResourceGroupManager.h>

using namespace Ogre;

namespace Opde {

	DTypeScriptLoader::DTypeScriptLoader() : mScriptPatterns() {
		mDTypeCompiler = new DTypeScriptCompiler();
		
		mScriptPatterns.push_back("*.dtype");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);
	}
			
	DTypeScriptLoader::~DTypeScriptLoader() {
		ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
		
		delete mDTypeCompiler;
	}
		
	const StringVector& DTypeScriptLoader::getScriptPatterns(void) const {
		return mScriptPatterns;
	}
		
 	void DTypeScriptLoader::parseScript(DataStreamPtr &stream, const String &groupName) {
		mDTypeCompiler->parseScript(stream, groupName);
	}
	
	Real DTypeScriptLoader::getLoadingOrder(void) const {
		return 1;
	}
			
 	
}

