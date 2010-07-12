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

#include "PhysicsService.h"
#include "PhysModels.h"
#include "logger.h"
#include "File.h"

#include "PhysSphereModel.h"
#include "PhysOBBModel.h"
#include "PhysBSPModel.h"

namespace Opde {
	/*----------------------------------------------------*/
	/*---------------------- PhysModels ------------------*/
	/*----------------------------------------------------*/
	PhysModels::PhysModels(PhysicsService* owner) : mOwner(owner) {
		
	};
	
	//------------------------------------------------------
	PhysModel* PhysModels::get(int objid) const {
		IDModelMap::const_iterator it = mModels.find(objid);
		
		if (it != mModels.end())
			return it->second;
		
		return NULL;
	};
	
	//------------------------------------------------------
	void PhysModels::addToStationary(PhysModel* mdl) {
		STUB_WARN_TXT("Does not handle any notification, even model is not informed");
		
		// TODO: is it in stationary already?
		mStationaryObjects.insert(mdl->getID());
	}
	
	//------------------------------------------------------
	bool PhysModels::isStationary(int objid) {
		return mStationaryObjects.find(objid) != mStationaryObjects.end();
	}
	
	//------------------------------------------------------
	void PhysModels::read(const FilePtr& tag, unsigned int physVersion) {
		STUB_WARN();
		
		uint32_t count;
		*tag >> count;
		
		// Moving objects
		while (count--) {
			// first, the object shape (volume type) is read.
			uint32_t voltype;
			int32_t id;
			
			*tag >> voltype >> id;

			PhysModel* mdl;
			
			switch (voltype) {
				case 0x00: // Sphere model
				case 0x02: // point model
				case 0x04: // Sphere Hat model
					mdl = new PhysSphereModel(id);
					break;
				case 0x01: // BSP model
					mdl = new PhysBSPModel(id);
					break;
				case 0x03: // OBB model
					mdl = new PhysOBBModel(id);
					break;
				
				default:
					LOG_ERROR("PhysModels::read: Unknown volume type %d", voltype);
					OPDE_EXCEPT("PhysModels::read", "Unknown volume type encountered");
			}
			
			
			mdl->read(tag, physVersion);
			mModels.insert(std::make_pair(id, mdl));
		}
		
		// The same code for Stationary...
		*tag >> count;
		while (count--) {
			// first, the object shape (volume type) is read.
			uint32_t voltype;
			int32_t id;
			
			*tag >> voltype >> id;

			PhysModel* mdl;
			
			switch (voltype) {
				case 0x00: // Sphere model
				case 0x02: // point model
				case 0x04: // Sphere Hat model
					mdl = new PhysSphereModel(id);
					break;
				case 0x01: // BSP model
					mdl = new PhysBSPModel(id);
					break;
				case 0x03: // OBB model
					mdl = new PhysOBBModel(id);
					break;
				
				default:
					LOG_ERROR("PhysModels::read: Unknown volume type %d", voltype);
					OPDE_EXCEPT("PhysModels::read", "Unknown volume type encountered");
			}
			
			
			mdl->read(tag, physVersion);
			mModels.insert(std::make_pair(id, mdl));
			addToStationary(mdl);
		}
		
	};
	
	//------------------------------------------------------
	void PhysModels::write(const FilePtr& tag, unsigned int physVersion) {
		STUB_WARN();
		// if the object is found in the stationary list, it is only written in the second pass
	};
	

	//------------------------------------------------------
	void PhysModels::clear(void) {
		STUB_WARN();
	}
	
}
