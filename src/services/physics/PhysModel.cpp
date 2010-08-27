/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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


#include "PhysModel.h"
#include "logger.h"
#include "File.h"
#include "FileCompat.h"

#include "PhysSphereModel.h"
#include "PhysOBBModel.h"
#include "PhysBSPModel.h"

namespace Opde {
	// helpers
	File& operator>>(File& f, PhysModel::Spring& s) {
		f >> s.tension >> s.damping;
		
		return f;
	}
	
	File& operator<<(File& f, PhysModel::Spring& s) {
		f << s.tension << s.damping;
		
		return f;
	}
	
	
	/*----------------------------------------------------*/
	/*----------------------- PhysModel ------------------*/
	/*----------------------------------------------------*/
	PhysModel::PhysModel(int objid) : mObjectID(objid), mSubModelTypes(NULL) {
		
	};
	
	//------------------------------------------------------
	PhysModel::~PhysModel()	{
		clear();
	}
	
	//------------------------------------------------------
	void PhysModel::read(const FilePtr& sf, unsigned int physVersion) {
		STUB_WARN();
		
		clear();
		
		*sf >> mObjectID >> mSubModelCount >> mFlags >> mGravity;
		
		// TODO: FaceVel, MovingTerrain and Door (2z) properties to set the flags
		mSubModelTypes = new uint32_t[mSubModelCount];
		
		for (size_t n = 0; n < mSubModelCount; ++n)
			*sf >> mSubModelTypes[n];
		
		*sf >> mFriction >> mMediaType;
		
		// spring parameters
		mSprings = new Spring[mSubModelCount];
		
		for (size_t n = 0; n < mSubModelCount; ++n) {
			*sf >> mSprings[n];
		}
		
		// some unknown 3 fields
		uint32_t unk;
		
		*sf >> mRopeVsTerrain;   // Rope v.s. terrain bool
		*sf >> mTime; // Dunno what exactly this is
		
		// attachment related (nonzero if attachments are in use)
		*sf >> mPhysAttachments;
		*sf >> mPhysAttached; // Could be bool, eh?
		
		// flags for rotation and resting
		*sf >> mRotAxes >> mRestAxes; 
		
		// Rope attachment.
		*sf >> mRopeAttObjID >> mRopeAttSubModel >> mRopeSegPos;
		
		// mantling state (T2 and such)
		if (physVersion >= 32) {
			*sf >> mMantlingState;
			*sf >> mMantlingVec;
		}
		
		// Dunno what this is...
		*sf >> unk >> unk;
		
		// SubModels...
		mSubModels.grow(mSubModelCount);
		
		for (unsigned int i = 0; i < mSubModelCount; ++i) {
			SubModel& sm = mSubModels[i]; 
			*sf >> sm;
			
			// fix the owner
			sm.owner = this;
		}
		
		*sf >> mMainSubModel;
		
		// Following are what seems to be relative submodel positions
		for (unsigned int i = 0; i < mSubModelCount; ++i) {
			*sf >> mRelPos[i];
		}
		
		// translation limit for pplate - if such exists
		if (mFlags & PHYS_MDL_PPLATE) {
			// see of there is a translation limit
			uint32_t stat;
			
			*sf >> stat;
			
			// this logic is in the original...
			if ((stat == 3) || (stat == 2)) {
				Vector3 transl;
				
				*sf >> transl;
				
				// TODO: addTranslationLimit(transl);
			}
		}
		
		// Dynamics follow
		
		// Rot vel. controls.
		
		// only after this block, the object type specific parameters are read - e,g, here
	}
	
	//------------------------------------------------------
	void PhysModel::write(const FilePtr& sf, unsigned int physVersion) {
		STUB_WARN();
		assert(mSubModelTypes);
		assert(mSprings);
	}
	
	//------------------------------------------------------
	int32_t PhysModel::getID() const {
		return mObjectID;
	}


	//------------------------------------------------------
	void PhysModel::updateMedium() {
		// TODO: Code
		STUB_WARN();
	}
	
	//------------------------------------------------------
	void PhysModel::setSleep(bool sleep) {
		STUB_WARN();
		/** TODO:
		* Contact PhysModels - de/reregister from phys updates (originally cPhysModels::StartMoving(cPhysModel*) / cPhysModels::StopMoving(cPhysModel*))
		* Original engine issues a series of vector zeroing operations (velocities both rotational and translational for all submodels)
		* Send an appropriate message
		* mIsAsleep?
		*/
	}
	
	//------------------------------------------------------
	void PhysModel::clear(void) {
		// Delete all the data used
		delete mSubModelTypes;
		mSubModelTypes = NULL;
		
		delete mSprings;
		mSprings = NULL;
		
		mSubModels.clear();
	}
}