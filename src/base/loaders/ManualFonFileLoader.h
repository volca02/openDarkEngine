/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *  $Id$
 *
 *****************************************************************************/

#ifndef __MANUALFONFILELOADER_H
#define __MANUALFONFILELOADER_H

#include <OgreResourceManager.h>
#include <OgreFont.h>
#include "File.h"
#include "FonFormat.h"

using namespace Opde; // For the Opde::File

namespace Ogre {
	
	/// ManualResourceLoader for FON files.
    class ManualFonFileLoader : public ManualResourceLoader 
	{		
        public:
			/// Palette type specifier
		    enum PaletteType {		    	
				ePT_Default = 0, /// Default palette				
				ePT_DefaultBook, /// Palette from accompanying BOOK.PCX				
				ePT_PCX,		 /// PCX file palette				
				ePT_External	 /// External palette
			};
            ManualFonFileLoader();
            virtual ~ManualFonFileLoader();

            virtual void loadResource(Resource* resource);
            
			/// Set the palette type
            void setPalette(PaletteType PalType = ePT_Default, String PalFileName = "");
            
		protected:			
			typedef std::map<String, String> Parameters;
			Parameters mParams;

		private:
			CharInfoList mChars;			
			DWORD mBmpFileSize;
			unsigned int mImageDim, mNumRows;
			FilePtr mFontFile, mBookFile, mPaletteFile;

			std::string mTxtName, mFontGroup; // the name of the dynamically generated texture
			
			PaletteType mPaletteType;
			String mPaletteFileName;

			RGBQUAD* ReadPalette();
			int CreateOgreFont(Font* DarkFont);
			int LoadDarkFont();
			int WriteImage(RGBQUAD *ColorTable, unsigned char **RowPointers);
			unsigned char** ReadFont(int *ResultingColor);

			void createOgreTexture(unsigned char** img, RGBQUAD* palette);
    };
}

#endif	//__MANUALFONFILELOADER_H
