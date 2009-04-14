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
 *
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __NULLTEXTUREMANAGER_H
#define __NULLTEXTUREMANAGER_H

#include "stdafx.h"

namespace Ogre {

	class NULLTextureManager : public TextureManager
	{
		protected:
			/// @copydoc ResourceManager::createImpl
			Resource* createImpl(const String& name, ResourceHandle handle, 
				const String& group, bool isManual, ManualResourceLoader* loader, 
				const NameValuePairList* createParams);
	
		public:
			NULLTextureManager();
			~NULLTextureManager();
	
			/// @copydoc TextureManager::getNativeFormat
			PixelFormat getNativeFormat(TextureType ttype, PixelFormat format, int usage);
	
			virtual bool isHardwareFilteringSupported (TextureType ttype, PixelFormat format, int usage, bool preciseFormatOnly=false);
	};

}

#endif
