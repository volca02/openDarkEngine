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

#ifndef __PHYSMODEL_H
#define __PHYSMODEL_H

#include "integers.h"
#include "File.h"

namespace Opde {
	/** @brief Physics model. Contains all data on physics for particular object
	*/
	class OPDELIB_EXPORT PhysModel {
		public:
			PhysModel(int objid);
			~PhysModel();
			
			virtual void read(const FilePtr& sf, unsigned int physVersion);
			virtual void write(const FilePtr& sf, unsigned int physVersion);
			
			/// @return the object id of this model
			int32_t getID() const;
			
			struct Spring {
				float tension;
				float damping;
			};

			//  - go find current cell via BSP, discover the media, update the media of the submodels
			// broadcast media changed event
			void updateMedium();
			
			/** puts a model to sleep or wakes it up
			* @note Should handle all the intricacies related to the fact object fell asleep including PhysFellAsleep/PhysWokeUp script messages
			* @note Will stop all the movement of the object
			* @param sleep The desired state of the object physical model (true will put to sleep, false will wake up)
			*/
			void setSleep(bool sleep);
			
		protected:
			
			int32_t mObjectID;
			uint32_t mSubModelCount;
			uint32_t mFlags;
			float mGravity;
			uint32_t *mSubModelTypes; // the same as the main type of the object
			float mFriction;
			/** Media type. Not entirelly the same as encoded in WR for some obscure reason
			* Mapping: 3->8, 2->1, 1->0, otherwise it stays the same
			*/
			int32_t mMediaType;
			Spring *mSprings;
			/// nonzero if the object has attachments
			uint32_t mPhysAttachments;
			/// nonzero if the object is attached
			uint32_t mPhysAttached;
			/// Rotational flags
			uint32_t mRotFlags;
			/// Resting flags
			uint32_t mRestFlags;
			/// Mantling state
			uint32_t mMantlingState;
	};
};


#endif
