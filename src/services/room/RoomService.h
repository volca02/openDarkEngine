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


#ifndef __ROOMSERVICE_H
#define __ROOMSERVICE_H

#include "config.h"
#include "RoomCommon.h"
#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DatabaseService.h"
#include "FileGroup.h"
#include "SharedPtr.h"

namespace Opde {
	/** @brief Room service - service providing a Room database.
	* Room database is a high level system that classifies object positions into 'rooms'.
	* @note This service listens to object positions 
	* @note This service is a source of script messages
	*/
	class OPDELIB_EXPORT RoomService : public ServiceImpl<RoomService>, DatabaseListener {
		public:
			/// Constructor
			RoomService(ServiceManager *manager, const std::string& name);
			
			/// Destructor
			virtual ~RoomService();

			Room* getRoomByID(int32_t id);
			
		protected:
			// service related
			bool init();
			void bootstrapFinished();
			void shutdown();
			
			void clear();
			
			/** Database load callback 
			* @see DatabaseListener::onDBLoad */
			void onDBLoad(const FileGroupPtr& db, uint32_t curmask);
			
			/** Database save callback 
			* @see DatabaseListener::onDBSave */
			void onDBSave(const FileGroupPtr& db, uint32_t tgtmask);
			
			/** Database drop callback 
			* @see DatabaseListener::onDBDrop */
			void onDBDrop(uint32_t dropmask);
		
		
		private:
			typedef std::map<int32_t, Room*> RoomsByID;
			
			/// Database service
			DatabaseServicePtr mDbService;

			/// Array of all rooms
			SimpleArray<Room*> mRooms;
			
			/// Map of rooms by their ID
			RoomsByID mRoomsByID;
			
			/// Indicates the room database is loaded/ok
			bool mRoomsOk;
	};

	/// Shared pointer to Room service
	typedef shared_ptr<RoomService> RoomServicePtr;


	/// Factory for the RoomService objects
	class OPDELIB_EXPORT RoomServiceFactory : public ServiceFactory {
		public:
			RoomServiceFactory();
			~RoomServiceFactory() {};

			/** Creates a RoomService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask(); 
			
			virtual const size_t getSID();
		private:
			static std::string mName;
	};
}


#endif
