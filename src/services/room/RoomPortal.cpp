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


#include "RoomPortal.h"
#include "Room.h"
#include "RoomService.h"
#include "FileCompat.h"

namespace Opde {
	/*----------------------------------------------------*/
	/*--------------------- RoomPortal -------------------*/
	/*----------------------------------------------------*/
	RoomPortal::RoomPortal(RoomService* owner) :
			mOwner(owner) {
		
	}
	
	//------------------------------------------------------
	RoomPortal::~RoomPortal() {
		clear();
	}
	
	//------------------------------------------------------
	void RoomPortal::read(const FilePtr& sf) {
		*sf >> mID >> mIndex >> mPlane >> mEdgeCount;
		// planes that define edges
		
		mEdges.grow(mEdgeCount);
		
		for (size_t i = 0; i < mEdgeCount;++i) {
			*sf >> mEdges[i];
		}
		
		// room ID's to be linked
		int32_t src_room, dest_room;
		
		*sf >> src_room >> dest_room;
		
		// link through room service
		mSrcRoom = mOwner->getRoomByID(src_room);
		mDestRoom = mOwner->getRoomByID(dest_room);
		
		*sf >> mCenter;
		*sf >> mDestPortal;
	}
	
	//------------------------------------------------------
	void RoomPortal::write(const FilePtr& sf) {
		*sf << mID << mIndex << mPlane << mEdgeCount;

		// planes that define edges
		for (size_t i = 0; i < mEdgeCount;++i) {
			*sf << mEdges[i];
		}
		
		*sf << mSrcRoom->getRoomID() << mDestRoom->getRoomID();
		*sf << mCenter;
		*sf << mDestPortal;
	}
	
	//------------------------------------------------------
	bool RoomPortal::isInside(const Ogre::Vector3& point) {
		// iterate over all the planes. Have to have positive side
		for (size_t i = 0; i < mEdgeCount;++i) {
			if (mEdges[i].getSide(point) == Ogre::Plane::NEGATIVE_SIDE)
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------
	void RoomPortal::clear() {
		mID = 0;
		mIndex = 0;
		mPlane = Ogre::Plane();
		mEdgeCount = 0;
		mEdges.clear();
		mSrcRoom = NULL;
		mDestRoom = NULL;
		mCenter = Ogre::Vector3::ZERO;
		mDestPortal = 0;
		
	}
}


