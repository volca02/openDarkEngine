/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/

#ifndef __BINMESHTFILEMANAGER_H
#define __BINMESHTFILEMANAGER_H

#include <OgreResourceManager.h>
#include "OgreMesh.h"

namespace Ogre {

    /** ManualResourceLoader for BIN meshes. Used to load BIN meshes as Ogre::Mesh instances */
    class ManualBinFileLoader : public ManualResourceLoader {
        public:
            ManualBinFileLoader();
            virtual ~ManualBinFileLoader();

            virtual void loadResource(Resource* resource);
    };

}

#endif
