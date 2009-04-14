/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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
 *	  $Id$
 *
 *****************************************************************************/

#include "NullGpuProgramManager.h"
#include "NullGpuProgram.h"

namespace Ogre {

//-----------------------------------------------------------------------------
NULLGpuProgramManager::NULLGpuProgramManager() :
	GpuProgramManager() {
	// Superclass sets up members 

	// Register with resource group manager
	ResourceGroupManager::getSingleton()._registerResourceManager(
			mResourceType, this);

}
//-----------------------------------------------------------------------------
NULLGpuProgramManager::~NULLGpuProgramManager() {
	// Unregister with resource group manager
	ResourceGroupManager::getSingleton()._unregisterResourceManager(
			mResourceType);

}
//-----------------------------------------------------------------------------
GpuProgramParametersSharedPtr NULLGpuProgramManager::createParameters(void) {
	return GpuProgramParametersSharedPtr(new GpuProgramParameters());
}
//-----------------------------------------------------------------------------
Resource* NULLGpuProgramManager::createImpl(const String& name,
		ResourceHandle handle, const String& group, bool isManual,
		ManualResourceLoader* loader, const NameValuePairList* params) {
	return new NULLGpuProgram(this, name, handle, group, isManual, loader);
}

//-----------------------------------------------------------------------------
Resource* NULLGpuProgramManager::createImpl(const String& name,
		ResourceHandle handle, const String& group, bool isManual,
		ManualResourceLoader* loader, GpuProgramType gptype,
		const String& syntaxCode) {
	return new NULLGpuProgram(this, name, handle, group, isManual, loader);
}

}
