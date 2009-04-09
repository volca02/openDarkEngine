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
