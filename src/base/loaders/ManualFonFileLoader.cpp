/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *	  Includes code based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>
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

#include "ManualFonFileLoader.h"
#include <OgreStringConverter.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreLogManager.h>
#include <OgreTextureManager.h>

using namespace std;
using namespace Opde; // For the Opde::File

namespace Ogre 
{


    /*-----------------------------------------------------------------*/
	/*--------------------- ManualFonFileLoader -----------------------*/
	/*-----------------------------------------------------------------*/
    ManualFonFileLoader::ManualFonFileLoader() : ManualResourceLoader() 
	{
    }

    //-------------------------------------------------------------------
    ManualFonFileLoader::~ManualFonFileLoader() 
	{
		delete [] mChars;
    }

	//-------------------------------------------------------------------
	char** ManualFonFileLoader::ReadFont(MemoryFile* MemFile, int *ResultingColor)
	{
		struct DarkFontHeader FontHeader;		
		unsigned int ImageWidth, ImageHeight, FinalSize = 1;
		unsigned int X, Y;
		unsigned int N, I;
		char *Ptr;

		MemFile->read(&FontHeader, sizeof(FontHeader));
		mNumChars = FontHeader.LastChar - FontHeader.FirstChar + 1;

		vector <unsigned short> Widths;
		MemFile->seek(FontHeader.WidthOffset, File::FSEEK_BEG);
		for(I = 0;I < mNumChars + 1; I++)
		{
			unsigned short Temp;
			MemFile->readElem(&Temp, sizeof(Temp));
			Widths.push_back(Temp);
		}

		char *BitmapData = new char[MemFile->size() + 1];
		if (!BitmapData)
			return NULL;
		MemFile->seek(FontHeader.BitmapOffset, File::FSEEK_BEG);
		MemFile->read(BitmapData, MemFile->size() - FontHeader.BitmapOffset);		
		if (FontHeader.BitmapOffset < FontHeader.WidthOffset)
		{
			delete [] BitmapData;
			return NULL;
		}

		mChars = new CharInfo[mNumChars];
		if (!mChars)
		{
			delete [] BitmapData;
			return NULL;
		}

		ImageHeight = ((mNumChars / 16) + 2) * (FontHeader.NumRows + 4);
		
		Y = (FontHeader.NumRows / 2) + 2;
		X = 2;
		ImageWidth = 2;
		for (N = 0; N < mNumChars; N++)
		{
			mChars[N].Code = FontHeader.FirstChar + N;
			mChars[N].Column = Widths[N];
			mChars[N].Width = Widths[N+1] - Widths[N];
			mChars[N].X = X;
			mChars[N].Y = Y;
			X += mChars[N].Width + 6;
			if ((N & 0xF) == 0xF)
			{
				Y += FontHeader.NumRows + 4;
				if (X > ImageWidth)
					ImageWidth = X;
				X = 2;
			}
		}
		if (X > ImageWidth)
			ImageWidth = X;

		while ((FinalSize < ImageWidth) || (FinalSize < ImageHeight))
			FinalSize <<= 1;

		char ** RowPointers = new char* [FinalSize];
		if (!RowPointers)
		{			
			delete [] BitmapData;
			return NULL;
		}

		char *ImageData = new char[FinalSize * FinalSize];
		if (!ImageData)
		{
			delete [] RowPointers;
			delete [] BitmapData;
			return NULL;
		}
		memset(ImageData, BLACK_INDEX, FinalSize * FinalSize);
		Ptr = ImageData;
		for (N = 0; N < FinalSize; N++)
		{
			RowPointers[N] = Ptr;
			Ptr += FinalSize;
		}

		if (FontHeader.Format == 0)
		{
			Ptr = BitmapData;
			for (I = 0; I < FontHeader.NumRows; I++)
			{
				for (N = 0; N < mNumChars; N++)
				{
					Y = mChars[N].Column;
					for (X = 0; X < mChars[N].Width; Y++, X++)
						RowPointers[mChars[N].Y + I][mChars[N].X + X] = ((Ptr[Y / 8]>>(7 - (Y % 8))) & 1) ? WHITE_INDEX : BLACK_INDEX;
				}
				Ptr += FontHeader.RowWidth;
			}
		}
		else
		{
			Ptr = BitmapData;
			for (I = 0; I < FontHeader.NumRows; I++)
			{
				for (N = 0; N < mNumChars; N++)
				{
					memcpy(RowPointers[mChars[N].Y + I]+mChars[N].X, Ptr, mChars[N].Width);				
					Ptr += mChars[N].Width;
				}
			}
		}
		
		mImageDim = FinalSize;
		switch (FontHeader.Format)
		{
		case 0:
			*ResultingColor = 1; 			
			break;

		case 1:
			*ResultingColor = 2; 
			break;

		case 0xCCCC:
			*ResultingColor = (FontHeader.Palette == 1) ? -1 : 0;
			break;

		default:
			*ResultingColor = 0;//Unknown pixel Format, assuming 8bpp.
			break;
		}

		delete [] BitmapData;
		return RowPointers;
	}


	//-------------------------------------------------------------------
	RGBQUAD* ManualFonFileLoader::ReadPalette(StdFile *FilePointer)
	{		
		return NULL;	//Add support for external palettes later.
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::WriteImage(StdFile* BitmapFile, RGBQUAD *ColorTable, char **RowPointers)
	{
		BITMAPFILEHEADER	FileHeader;
		BITMAPINFOHEADER	BitmapHeader;
		int 	RowWidth, Row;
		char	Zero[4] = {0,0,0,0};

		FileHeader.bfType = 0x4D42;
		FileHeader.bfReserved1 = 0;
		FileHeader.bfReserved2 = 0;
		FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*256);
		BitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
		BitmapHeader.biWidth = mImageDim;
		BitmapHeader.biHeight = mImageDim;
		BitmapHeader.biPlanes = 1;
		BitmapHeader.biBitCount = 8;
		BitmapHeader.biCompression = 0;
		BitmapHeader.biXPelsPerMeter = 0xB13;
		BitmapHeader.biYPelsPerMeter = 0xB13;
		BitmapHeader.biClrUsed = 256;
		BitmapHeader.biClrImportant = 256;
		RowWidth = (mImageDim + 3) & ~3;
		BitmapHeader.biSizeImage = RowWidth * mImageDim;
		FileHeader.bfSize = FileHeader.bfOffBits + BitmapHeader.biSizeImage;

		BitmapFile->write(&FileHeader, sizeof(BITMAPFILEHEADER));
		BitmapFile->write(&BitmapHeader, sizeof(BITMAPINFOHEADER));
		BitmapFile->write(ColorTable, sizeof(RGBQUAD) * 256);

		RowWidth -= mImageDim;
		for (Row = mImageDim - 1; Row >= 0; Row--)
		{
			BitmapFile->write(RowPointers[Row], mImageDim);
			if (RowWidth != 0)
				BitmapFile->write(Zero, RowWidth);
		}
		return 0;
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::LoadFont(MemoryFile* MemFile, String PaletteFileName)
	{
		COLORREF *PaletteData;
		char **ImageRows;
		int Color;

		/*if (PaletteFileName != "")
		{
			StdFile *PaletteFile = new StdFile(PaletteFileName, File::FILE_R);
			if (PaletteFile == 0)
			{
				LogManager::getSingleton().logMessage(PaletteFileName + " not found.");
				return -1;
			}
			PaletteData = (COLORREF*)ReadPalette(PaletteFile);
			delete PaletteFile;
			if (!PaletteData)
			{
				LogManager::getSingleton().logMessage("Invalid palette file : " + PaletteFileName);
				return -2;
			}
		}
		else*/
			PaletteData = (COLORREF*)ColorTable;

		ImageRows = ReadFont(MemFile, &Color);
		if (!ImageRows)
		{
			//if (PaletteData != ColorTable)
			//	free(PaletteData);
			return 2;
		}
		if (Color == 2 && PaletteData == ColorTable)
			PaletteData = (COLORREF*)AntiAliasedColorTable;

		StdFile* BitmapFile = new StdFile("Font.bmp", File::FILE_W);		
		WriteImage(BitmapFile, (RGBQUAD*)PaletteData, ImageRows);
		delete BitmapFile;

		for (unsigned int N = 0; N < mImageDim; N++)
			delete [] ImageRows[N];
		delete [] ImageRows;
		//if (PaletteData != ColorTable && PaletteData != AntiAliasedColorTable)
		//	free(PaletteData);
		return 0;
	}

	
	//-------------------------------------------------------------------
    void ManualFonFileLoader::loadResource(Resource* resource) 
	{
        // Cast to font, and fill
        Font* DarkFont = static_cast<Font*>(resource);

        // Fill. Find the file to be loaded by the name, and load it
        String FontName = DarkFont->getName();
        String FontGroup = DarkFont->getGroup();

        // Get the real filename from the nameValuePairList
        // That means: truncate to the last dot, append .fon to the filename
        size_t DotPos = FontName.find_last_of(".");

        String BaseName = FontName;
        if (DotPos != String::npos){
            BaseName = FontName.substr(0, DotPos);
        }

        BaseName += ".fon";

        //Open the file, and detect the mesh type (Model/AI)
        Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(BaseName, DarkFont->getGroup(), true, resource);

        FilePtr FontFile = new OgreFile(Stream);

        MemoryFile* MemFile = new MemoryFile(BaseName, File::FILE_R);
        MemFile->initFromFile(*FontFile, FontFile->size()); // read the whole contents into the memory file

        if(LoadFont(MemFile, ""))
			LogManager::getSingleton().logMessage("An error happened while loading the font " + BaseName);
		delete MemFile;

		//  OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Unknown FON model format : '") + header + "'", "ManualFonFileLoader::loadResource");
    }
}


