/*
Copied from the Ogre3d wiki: Article "Per renderable transparency"
Edited to fit the usage in open dark engine

$Id$

*/

#include "SubEntityMaterialInstance.h"

#include <OgreSubEntity.h>

using namespace Ogre;

SubEntityMaterialInstance::SubEntityMaterialInstance (SubEntity *se) : MaterialInstance () {
	mSubEntity = se;
      
	initOriginalMaterial ();
}

SubEntityMaterialInstance::~SubEntityMaterialInstance () {
	// Reset to the original material
	mSubEntity->setMaterialName (mOriginalMat->getName ());
}

void SubEntityMaterialInstance::setMaterialName (String name) {
	clearCopyMaterial ();
      
	mSubEntity->setMaterialName (name);
	  
	initOriginalMaterial ();
	  
	setTransparency (mCurrentTransparency);
}

void SubEntityMaterialInstance::setTransparency (Real transparency) {
	MaterialInstance::setTransparency (transparency);

	if (hasOverrides()) {
		mSubEntity->setMaterialName (mCopyMat->getName ());
	} else {
		mSubEntity->setMaterialName (mOriginalMat->getName ());
	}
}

void SubEntityMaterialInstance::setZBias (Ogre::Real zbias) {
	MaterialInstance::setZBias (zbias);

	if (hasOverrides()) {
		mSubEntity->setMaterialName (mCopyMat->getName ());
	} else {
		mSubEntity->setMaterialName (mOriginalMat->getName ());
	}
}

void SubEntityMaterialInstance::initOriginalMaterial () {
	mOriginalMat = MaterialManager::getSingleton ().getByName (mSubEntity->getMaterialName ());
}

