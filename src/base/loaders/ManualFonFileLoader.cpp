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
 *****************************************************************************/

//---------------------------- El cargador de los fuentes para El Motor Oscuro --------------------

#include "FreeImage.h"
#include "ManualFonFileLoader.h"
#include <OgreLogManager.h>

using namespace std;
using namespace Opde; //For the Opde::File

namespace Ogre 
{
    /*-----------------------------------------------------------------*/
	/*--------------------- ManualFonFileLoader -----------------------*/
	/*-----------------------------------------------------------------*/
    ManualFonFileLoader::ManualFonFileLoader() : ManualResourceLoader() 
	{
		mChars = NULL;
		mMemBuff = NULL;
    }

    //-------------------------------------------------------------------
    ManualFonFileLoader::~ManualFonFileLoader() 
	{
		if(!mChars)
			delete [] mChars;
		if(!mMemBuff)
			delete [] mMemBuff;
    }


	/*-----------------------------------------------------------------*/
	/*------------------------ Bitmap stuff ---------------------------*/
	/*-----------------------------------------------------------------*/
	char** ManualFonFileLoader::ReadFont(MemoryFile* MemFile, int *ResultingColor)
	{
		struct DarkFontHeader FontHeader;		
		unsigned int ImageWidth, ImageHeight, FinalSize = 1;
		unsigned int X, Y;
		unsigned int N, I;
		char *Ptr;

		MemFile->readStruct(&FontHeader, DarkFontHeader_Format, sizeof(DarkFontHeader));
		mNumChars = FontHeader.LastChar - FontHeader.FirstChar + 1;
		mNumRows = FontHeader.NumRows;

		vector <unsigned short> Widths;
		MemFile->seek(FontHeader.WidthOffset, File::FSEEK_BEG);
		for(I = 0; I < mNumChars + 1; I++)
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
			mChars[N].Width = Widths[N + 1] - Widths[N];
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
					memcpy(RowPointers[mChars[N].Y + I] + mChars[N].X, Ptr, mChars[N].Width);				
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
	int ManualFonFileLoader::WriteImage(RGBQUAD *ColorTable, char **RowPointers)
	{
		BITMAPFILEHEADER	FileHeader;
		BITMAPINFOHEADER	BitmapHeader;
		int 	RowWidth, Row;
		char	Zero[4] = {0,0,0,0};

		mBmpFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256) + (mImageDim * mImageDim);
		mMemBuff = new unsigned char[mBmpFileSize];
		if(!mMemBuff)
			return -1;
		unsigned char *Ptr = mMemBuff;

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

		memcpy(Ptr, &FileHeader, sizeof(BITMAPFILEHEADER));
		Ptr += sizeof(BITMAPFILEHEADER);

		memcpy(Ptr, &BitmapHeader, sizeof(BITMAPINFOHEADER));
		Ptr += sizeof(BITMAPINFOHEADER);

		memcpy(Ptr, ColorTable, sizeof(RGBQUAD) * 256);
		Ptr += sizeof(RGBQUAD) * 256;

		RowWidth -= mImageDim;
		for (Row = mImageDim - 1; Row >= 0; Row--)
		{
			memcpy(Ptr, RowPointers[Row], mImageDim);
			Ptr += mImageDim;

			if (RowWidth != 0)
			{
				memcpy(Ptr, Zero, mImageDim);
				Ptr += RowWidth;
			}
		}

		//Uncommenting the following block will generate the intermediate bitmap file
		/*StdFile* BitmapFile = new StdFile("Font.bmp", File::FILE_W);
		BitmapFile->write(mMemBuff, mBmpFileSize);
		delete BitmapFile;*/

		return 0;
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::LoadDarkFont(MemoryFile* MemFile, String PaletteFileName)
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

		WriteImage((RGBQUAD*)PaletteData, ImageRows);		

		delete [] ImageRows;
		//if (PaletteData != ColorTable && PaletteData != AntiAliasedColorTable)
		//	free(PaletteData);
		return 0;
	}

	/*-----------------------------------------------------------------*/
	/*------------------------- Alpha stuff ---------------------------*/
	/*-----------------------------------------------------------------*/	
	int ManualFonFileLoader::AddAlpha()
	{
#ifdef FREEIMAGE_LIB
		FreeImage_Initialise();
#endif //FREEIMAGE_LIB

		FIMEMORY *BmpMem = FreeImage_OpenMemory(mMemBuff, mBmpFileSize);

		FIBITMAP *Src = FreeImage_LoadFromMemory(FIF_BMP, BmpMem, 0);
		if(!Src)
			return -1;

		FIBITMAP *Dst = FreeImage_ConvertTo32Bits(Src);

		FreeImage_Invert(Src);
		FIBITMAP *Mask = FreeImage_ConvertToGreyscale(Src);
		FreeImage_Invert(Src);
		FreeImage_SetChannel(Dst, Mask, FICC_ALPHA);
		FreeImage_Unload(Mask);

		FreeImage_Save(FIF_PNG, Dst, "Font.PNG");

		FreeImage_Unload(Dst);
		FreeImage_Unload(Src);		
		FreeImage_CloseMemory(BmpMem);		

#ifdef FREEIMAGE_LIB
		FreeImage_DeInitialise();
#endif //FREEIMAGE_LIB

		return 0;
	}


	/*-----------------------------------------------------------------*/
	/*------------------------- Ogre stuff ----------------------------*/
	/*-----------------------------------------------------------------*/
	int ManualFonFileLoader::CreateOgreFont(Font* DarkFont)
	{
		DarkFont->setSource("Font.PNG");
		DarkFont->setType(FT_IMAGE);
		for(unsigned int I = 0; I < mNumChars; I++)
		{
			if(isalnum(mChars[I].Code))
				DarkFont->setGlyphTexCoords(mChars[I].Code, (float)(mChars[I].X - 2) / mImageDim,
					(float)(mChars[I].Y - 2) / mImageDim, (float)(mChars[I].X + mChars[I].Width + 2) / mImageDim,
					(float)(mChars[I].Y + mNumRows + 2) / mImageDim, 1.0);
		}		
		DarkFont->load();			//Let's rock!

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

        //Open the file
        Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(BaseName, DarkFont->getGroup(), true, resource);

        FilePtr FontFile = new OgreFile(Stream);

        MemoryFile* MemFile = new MemoryFile(BaseName, File::FILE_R);
        MemFile->initFromFile(*FontFile, FontFile->size());
        if(LoadDarkFont(MemFile, ""))
			LogManager::getSingleton().logMessage("An error happened while loading the font " + BaseName);
		delete MemFile;

		if(AddAlpha())
			LogManager::getSingleton().logMessage("An error happened while adding Alpha Channel");

		if(CreateOgreFont(DarkFont))
			LogManager::getSingleton().logMessage("An error happened creating Ogre font");
    }
}
