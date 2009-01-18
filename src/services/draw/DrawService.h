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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __DRAWSERVICE_H
#define __DRAWSERVICE_H

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "DrawSheet.h"
#include "Array.h"

#include <stack>

namespace Opde {

	/** @brief Draw Service - 2D rendering service.
	*/
	class OPDELIB_EXPORT DrawService : public Service {
		public:
			DrawService(ServiceManager *manager, const std::string& name);
			virtual ~DrawService();

			/** Creates a new DrawSheet and inserts it into internal map.
			 * @param sheetName a unique sheet name
			 * @return new DrawSheet if none of that name exists, or the currently existing if sheet with that name already found
			 * */
			DrawSheet* createSheet(const std::string& sheetName);

			/** Destroys the given sheet, and removes it from the internal map
			 * @param sheet The sheet to destroy
			 */
			void destroySheet(DrawSheet* sheet);

			/** Returns the sheet of the given name
			 * @param sheetName the name of the sheet to return
			 * @return The sheet pointer, or NULL if none of that name exists
			 */
			DrawSheet* getSheet(const std::string& sheetName) const;

			/** Sets the active (currently displayed) sheet.
			 * @param sheet The sheet to display (or none if the parameter is NULL)
			 */
			void setActiveSheet(DrawSheet* sheet);


			/** Creates a DrawSource that represents a specified image.
			 * Also creates a material that is used to render the image.
			 * @param img The image name
			 * @return Shared ptr to the draw source usable for draw operations
			 */
			DrawSourcePtr& createDrawSource(const std::string& img, const std::string& group);

			/** Creates a rendered image (e.g. a sprite)
			 * @param draw The image source for this operation
			 */
			// RenderedImage* createRenderedImage(DrawSourcePtr& draw);

			/** Destroys the specified draw operation (any ancestor)
			 * @param dop The draw operation to destroy
			 */
			void destroyDrawOperation(DrawOperation* dop);

		protected:
			bool init();
			void bootstrapFinished();

			DrawOperation::ID getNewDrawOperationID();

			typedef std::map<std::string, DrawSheet*> SheetMap;
			typedef std::stack<size_t> IDStack;
			typedef SimpleArray<DrawOperation*> DrawOperationArray;

			SheetMap mSheetMap;
			DrawSheet* mActiveSheet;
			DrawOperation::ID mDrawOpID;
			IDStack mFreeIDs;
			DrawOperationArray mDrawOperations;
	};

	/// Shared pointer to the draw service
	typedef shared_ptr<DrawService> DrawServicePtr;


	/// Factory for the DrawService objects
	class OPDELIB_EXPORT DrawServiceFactory : public ServiceFactory {
		public:
			DrawServiceFactory();
			~DrawServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();
		private:
			static std::string mName;
	};
}


#endif
