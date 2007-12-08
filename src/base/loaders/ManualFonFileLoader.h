/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *	  Contains code based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>
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
	
	/** ManualResourceLoader for FON files. 
	* Font resource parameters:
	* Palette type: Paramater <b>palette_type</b>
	* @li external External Palette File (RIFF)
	* @li pcx PCX file palette
	* @li default Default (LG's default) palette
	* Palette file name paramater: <b>palette_file</b>
	*/
    class ManualFonFileLoader : public ManualResourceLoader {
		protected:
		    /// Internal - Palette type specifier
		    enum PaletteType {
		    	/// Default palette
				PT_Default = 0, 
				/// PCX file palette
				PT_PCX, 
				/// External palette
				PT_External
			};
			
			typedef std::map<String, String> Parameters;
			Parameters mParams;

		private:
			CharInfoList mChars;
			unsigned char *mpMemBuff;
			DWORD mBmpFileSize;
			unsigned int mImageDim, mNumRows;
			FilePtr mFontFile, mBookFile, mPaletteFile;

			std::string mTxtName, mFontGroup; // the name of the dynamically generated texture
			
			PaletteType mPaletteType;
			String mPaletteFileName;

			RGBQUAD* ReadPalette();
			int AddAlpha();
			int CreateOgreFont(Font* DarkFont);
			int LoadDarkFont();
			int WriteImage(RGBQUAD *ColorTable, unsigned char **RowPointers);
			unsigned char** ReadFont(int *ResultingColor);

			void createOgreTexture(unsigned char** img, RGBQUAD* palette);
		

        public:
            ManualFonFileLoader();
            virtual ~ManualFonFileLoader();

            virtual void loadResource(Resource* resource);
            
            /// Will set a parameter value
            void setParameter(String name, String value);
            
            /// Will set a parameter value
            const String getParameter(String name);
            
            /// Will clean the parameter list - always use this one before loading a new font
            void resetParameters();
    };

}

#endif	//__MANUALFONFILELOADER_H
