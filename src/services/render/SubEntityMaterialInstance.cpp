/*
Copied from the Ogre3d wiki: Article "Per renderable transparency"
Edited to fit the usage in open dark engine

$Id$

*/

#include "SubEntityMaterialInstance.h"
#include "logger.h"

#include <OgreSubEntity.h>

using namespace Opde;
using namespace Ogre;

SubEntityMaterialInstance::SubEntityMaterialInstance(SubEntity *se)
    : MaterialInstance() {
    mSubEntity = se;

    initOriginalMaterial();
}

SubEntityMaterialInstance::~SubEntityMaterialInstance() {
    // Reset to the original material
    mSubEntity->setMaterial(mOriginalMat);
}

void SubEntityMaterialInstance::setTransparency(Real transparency) {
    MaterialInstance::setTransparency(transparency);

    if (hasOverrides()) {
        mSubEntity->setMaterial(mCopyMat);
    } else {
        mSubEntity->setMaterial(mOriginalMat);
    }
}

void SubEntityMaterialInstance::setZBias(Ogre::Real zbias) {
    MaterialInstance::setZBias(zbias);

    if (hasOverrides()) {
        mSubEntity->setMaterial(mCopyMat);
    } else {
        mSubEntity->setMaterial(mOriginalMat);
    }
}

void SubEntityMaterialInstance::initOriginalMaterial() {
    mOriginalMat = mSubEntity->getMaterial();
}
