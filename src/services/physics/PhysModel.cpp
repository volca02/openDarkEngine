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
		// Delete all the data used
		delete mSubModelTypes;
		mSubModelTypes = NULL;
		
		delete mSprings;
		mSprings = NULL;
	}
	
	//------------------------------------------------------
	void PhysModel::read(const FilePtr& sf, unsigned int physVersion) {
		STUB_WARN();
		
		*sf >> mObjectID >> mSubModelCount >> mFlags >> mGravity;
		
		delete[] mSubModelTypes;
		mSubModelTypes = new uint32_t[mSubModelCount];
		
		for (size_t n = 0; n < mSubModelCount; ++n)
			*sf >> mSubModelTypes[n];
		
		*sf >> mFriction >> mMediaType;
		
		// spring parameters
		delete[] mSprings;
		mSprings = new Spring[mSubModelCount];
		
		for (size_t n = 0; n < mSubModelCount; ++n) {
			*sf >> mSprings[n];
		}
		
		// some unknown 3 fields
		uint32_t unk;
		*sf >> unk;
		*sf >> unk;
		
		// attachment related (nonzero if attachments are in use)
		*sf >> mPhysAttachments;
		*sf >> mPhysAttached;
		
		// flags for rotation and resting
		*sf >> mRotFlags >> mRestFlags;
		
		// TODO: another set of attachment data
		*sf >> unk >> unk >> unk;
		
		// mantling state (T2 and such)
		if (physVersion >= 32) {
			*sf >> mMantlingState;
		}
		
		// TODO: Unknowns: 4 floats, one integer
		*sf >> unk >> unk >> unk >> unk >> unk;
		
		
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
}