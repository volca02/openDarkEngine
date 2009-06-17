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
 *    $Id$
 *
 *****************************************************************************/

//---------------------------- El cargador de las fuentes para El Motor Oscuro --------------------
#include "config.h"
#include "ManualFonFileLoader.h"

#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreException.h>

using namespace std;
using namespace Opde; //For the Opde::File

// Both horizontal and vertical spacing for characters
#define _SPACING 3

namespace Ogre 
{

    const char BLACK_INDEX = 0;
    const char WHITE_INDEX = 1;

    /*-----------------------------------------------------------------*/
	/*--------------------- ManualFonFileLoader -----------------------*/
	/*-----------------------------------------------------------------*/
    ManualFonFileLoader::ManualFonFileLoader() : ManualResourceLoader(), mChars()
	{
		mPaletteFileName = "";
		mPaletteType = ePT_Default;
    }

    //-------------------------------------------------------------------
    ManualFonFileLoader::~ManualFonFileLoader() 
	{
    }


	/*-----------------------------------------------------------------*/
	/*------------------------ Bitmap stuff ---------------------------*/
	/*-----------------------------------------------------------------*/
	unsigned char** ManualFonFileLoader::ReadFont(int *ResultingColor)
	{
		struct DarkFontHeader FontHeader;
		unsigned int ImageWidth, ImageHeight, FinalSize = 1;
		unsigned int X, Y;
		unsigned int N;
		unsigned char *Ptr;
		unsigned int NumChars;

		mChars.clear();

		mFontFile->readStruct(&FontHeader, DarkFontHeader_Format, sizeof(DarkFontHeader));
		NumChars = FontHeader.LastChar - FontHeader.FirstChar + 1;
		mNumRows = FontHeader.NumRows;

		if (NumChars < 0)
			return NULL;

		vector <unsigned short> Widths;
		mFontFile->seek(FontHeader.WidthOffset, File::FSEEK_BEG);
		for(N = 0; N < NumChars + 1; N++)
		{
			unsigned short Temp;
			mFontFile->readElem(&Temp, sizeof(Temp));
			Widths.push_back(Temp);
		}

		if (FontHeader.BitmapOffset < FontHeader.WidthOffset)
			return NULL;

		unsigned char *BitmapData = new unsigned char[mFontFile->size() + 1];
		
		if (!BitmapData)
			return NULL;
		

		mFontFile->seek(FontHeader.BitmapOffset, File::FSEEK_BEG);
		mFontFile->read(BitmapData, mFontFile->size() - FontHeader.BitmapOffset);		
		
		// No precalc. Let's organise the Font into groups of 16, the resolution will come out

		// No more spacing than needed
		Y = 0;
		X = 0;
		ImageWidth  = 0;
		ImageHeight = 0;

		for (N = 0; N < NumChars; N++)
		{
			CharInfo Char;
			
			Char.Code = FontHeader.FirstChar + N;
			// Seems more like X coordinates if the font was in one row...
			Char.Column = Widths[N];
			Char.Width = Widths[N + 1] - Widths[N];
			Char.X = X;
			Char.Y = Y;
			
			X += Char.Width + _SPACING; // Occupy only what you need!
			
			if ((N & 0xF) == 0xF)
			{
				Y += FontHeader.NumRows + _SPACING; // Font Height + Spacing
			
				if (X > ImageWidth) // Sure, for the first time
					ImageWidth = X; // Update the maximal Width
					
				X = 0; // reset the X coord...
			}
			
			mChars.push_back(Char);
		}
		
		ImageHeight = Y + FontHeader.NumRows + _SPACING;

		// If the process did not finish with the 16th char in the row, try to update the width
		if (X > ImageWidth)
			ImageWidth = X;

		while ((FinalSize < ImageWidth) || (FinalSize < ImageHeight))
			FinalSize <<= 1;

		unsigned char ** RowPointers = new unsigned char* [FinalSize];
		if (!RowPointers)
		{
			delete [] BitmapData;
			return NULL;
		}

		unsigned char *ImageData = new unsigned char[FinalSize * FinalSize];
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
			for (N = 0; N < FontHeader.NumRows; N++)
			{
				for (CharInfoList::const_iterator It = mChars.begin(); It != mChars.end(); It++)
				{
					const CharInfo& Char = *It;
					Y = Char.Column;
					for (X = 0; X < Char.Width; Y++, X++)
						RowPointers[Char.Y + N][Char.X + X] = ((Ptr[Y / 8]>>(7 - (Y % 8))) & 1) ? WHITE_INDEX : BLACK_INDEX;
				}
				Ptr += FontHeader.RowWidth;
			}
		}
		else
		{
			Ptr = BitmapData;
			for (N = 0; N < FontHeader.NumRows; N++) // Scanline of the font character...
			{
				for (CharInfoList::const_iterator It = mChars.begin(); It != mChars.end(); It++)
				{
					const CharInfo& Char = *It;

					memcpy(RowPointers[Char.Y + N] + Char.X, Ptr, Char.Width);				
					Ptr += Char.Width;
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
	RGBQUAD* ManualFonFileLoader::ReadPalette()
	{
		ExternalPaletteHeader PaletteHeader;
		WORD Count;
		char *Buffer, *C;
		BYTE S;
		unsigned int I;

		// Open the file
		Ogre::DataStreamPtr Stream;
		
		try 
		{
			Stream = Ogre::ResourceGroupManager::getSingleton().openResource(mPaletteFileName, mFontGroup, true);
			mPaletteFile = FilePtr(new OgreFile(Stream));
		} catch(Ogre::FileNotFoundException) {
			// Could not find resource, use the default table
			LogManager::getSingleton().logMessage("Specified palette file not found - using default palette!");
			return (RGBQUAD*)ColorTable;
		}
		
		RGBQUAD *Palette = new RGBQUAD[256];
		if (!Palette)
			return NULL;
		
		if((mPaletteType == ePT_PCX) || (mPaletteType == ePT_DefaultBook))	// PCX file specified...
		{
			RGBQUAD *Palette = new RGBQUAD[256];
			
			// Test to see if we're facing a PCX file. (0A) (xx) (01)
			uint8_t Manuf, Enc;
			mPaletteFile->read(&Manuf, 1);
			mPaletteFile->seek(2);
			mPaletteFile->read(&Enc, 1);
			
			if (Manuf != 0x0A || Enc != 0x01) // Invalid file, does not seem like a PCX at all
			{ 
				delete[] Palette; // Clean up!
				LogManager::getSingleton().logMessage("Invalid palette file specified - seems not to be a PCX file!");
				return (RGBQUAD*)ColorTable; // Should not matter - the cast (if packed)
			}
			
			BYTE BPP;
			mPaletteFile->readElem(&BPP, 1);
			
			mPaletteFile->seek(3 * 256 + 1, File::FSEEK_END);
			BYTE Padding;
			mPaletteFile->readElem(&Padding, 1);
			
			if((BPP == 8) && (Padding == 0x0C)) //Make sure it is an 8bpp and a valid PCX
			{
				// Byte sized structures - endianness always ok
				for (unsigned int I = 0; I < 256; I++)
				{
					
					mPaletteFile->read(&Palette[I].rgbRed, 1);
					mPaletteFile->read(&Palette[I].rgbGreen, 1);
					mPaletteFile->read(&Palette[I].rgbBlue, 1);
					Palette[I].rgbReserved = 0;
				}
			} else 
			{
				delete[] Palette; // Clean up!
				LogManager::getSingleton().logMessage("Invalid palette file specified - not 8 BPP or invalid Padding!");
				return (RGBQUAD*)ColorTable; // Return default palette
			}			
			return Palette;
		}
		
		if (mPaletteType != ePT_External) 
		{
			delete[] Palette; // Clean up!
			LogManager::getSingleton().logMessage("Invalid palette type specified!");
			return (RGBQUAD*)ColorTable;
		}
		
		// We're sure that we have external palette here:
		mPaletteFile->readStruct(&PaletteHeader, ExternalPaletteHeader_Format, sizeof(ExternalPaletteHeader));
		if (PaletteHeader.RiffSig == 0x46464952)
		{
			if (PaletteHeader.PSig1 != 0x204C4150)
			{
				delete []Palette;
				return NULL;
			}
			mPaletteFile->seek(2L, StdFile::FSEEK_CUR);
			mPaletteFile->readElem(&Count, 2);
			if (Count > 256)
				Count = 256;
			for (I = 0; I < Count; I++)
			{
				mPaletteFile->readStruct(&Palette[I], RGBQUAD_Format, sizeof(RGBQUAD));
				S = Palette[I].rgbRed;
				Palette[I].rgbRed = Palette[I].rgbBlue;
				Palette[I].rgbBlue = S;
			}
		}
		else if (PaletteHeader.RiffSig == 0x4353414A)
		{
			mPaletteFile->seek(0);
			Buffer = new char[3360];
			if (!Buffer)
			{
				delete []Palette;
				return NULL;
			}
			mPaletteFile->read(Buffer, 3352);
			if (strncmp(Buffer, "JASC-PAL", 8))
			{
				delete []Buffer;
				delete []Palette;
				return NULL;
			}
			C = strchr(Buffer, '\n')+1;
			C = strchr(C, '\n')+1;
			Count = (WORD)strtoul(C, NULL, 10);
			if (Count > 256)
				Count = 256;
			for (I = 0; I < Count; I++)
			{
				C = strchr(C, '\n')+1;
				Palette[I].rgbRed = (BYTE)strtoul(C, &C, 10);
				C++;
				Palette[I].rgbGreen = (BYTE)strtoul(C, &C, 10);
				C++;
				Palette[I].rgbBlue = (BYTE)strtoul(C, &C, 10);
			}
			delete []Buffer;
		}
		else
		{
			mPaletteFile->seek(0);
			RGBTRIPLE P;
			for (I = 0; I < mPaletteFile->size() / sizeof(RGBTRIPLE); I++)
			{
				mPaletteFile->readStruct(&P, RGBTRIPLE_Format, sizeof(RGBTRIPLE));
				Palette[I].rgbRed = P.rgbtBlue;
				Palette[I].rgbGreen = P.rgbtGreen;
				Palette[I].rgbBlue = P.rgbtRed;
			}
		}
		return Palette;
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::WriteImage(RGBQUAD *ColorTable, unsigned char **RowPointers)
	{
		BITMAPFILEHEADER	FileHeader;
		BITMAPINFOHEADER	BitmapHeader;
		int 	RowWidth, Row;
		char	Zero[4] = {0,0,0,0};
		unsigned char *pMemBuff;

		mBmpFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256) + (mImageDim * mImageDim);
		pMemBuff = new unsigned char[mBmpFileSize];
		if(!pMemBuff)
			return -1;
		unsigned char *Ptr = pMemBuff;

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

		StdFile* BitmapFile = new StdFile("Font.bmp", File::FILE_W);
		BitmapFile->write(pMemBuff, mBmpFileSize);
		delete BitmapFile;
		delete [] pMemBuff;

		return 0;
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::LoadDarkFont()
	{
		COLORREF *PaletteData = const_cast<COLORREF*>(ColorTable);
		unsigned char **ImageRows;
		int Color;

		// Default palette
		
		if (mPaletteType != ePT_Default) {
			PaletteData = (COLORREF*)ReadPalette();
			
			if (!PaletteData) {
				LogManager::getSingleton().logMessage("Could not load palette data, defaulting to default palette");
				// return -2;
				PaletteData = const_cast<COLORREF*>(ColorTable);
			}
		}
		
		ImageRows = ReadFont(&Color);
		
		if (!ImageRows)
		{
			if (PaletteData != ColorTable)
				delete []PaletteData;
			return 2;
		}
		
		if (Color == 2 && PaletteData == ColorTable) {
			if (PaletteData != ColorTable)
				delete[] PaletteData; // Clean up!
				
			PaletteData = (COLORREF*)AntiAliasedColorTable;
		}

		// Enable to see the resulting BMP
		// WriteImage((RGBQUAD*)PaletteData, ImageRows);

		createOgreTexture(ImageRows, (RGBQUAD*)PaletteData);


		delete [] ImageRows;
		if (PaletteData != ColorTable && PaletteData != AntiAliasedColorTable)
			delete []PaletteData;
		return 0;
	}

    //-------------------------------------------------------------------
	void ManualFonFileLoader::createOgreTexture(unsigned char** Img, RGBQUAD* Palette) 
	{
		// Create a texure, then fill it
		TexturePtr Txt = TextureManager::getSingleton().createManual(mTxtName, mFontGroup, TEX_TYPE_2D, mImageDim, mImageDim, 1, PF_A8R8G8B8);

		// Lock the texture, obtain a pointer
		HardwarePixelBufferSharedPtr pBuf = Txt->getBuffer(0, 0);

		// Erase the lmap atlas pixel buffer
		pBuf->lock(HardwareBuffer::HBL_DISCARD);
		const PixelBox &pB = pBuf->getCurrentLock();

		// Copy the image data, converting to 32bit on the fly...
		for (unsigned int Y = 0; Y < mImageDim; Y++)
		{
		    unsigned char* Row = Img[Y];
			uint32 *Data = static_cast<uint32*>(pB.data) + Y * pB.rowPitch;

			for (unsigned int X = 0; X < mImageDim; X++) 
			{
				int PalIdx = Row[X];

				unsigned char Red , Green, Blue, Alpha;

                Alpha = 255;
				if (PalIdx == BLACK_INDEX)
                    Alpha = 0;

				// palidx = 255 - palidx;

                Red = Palette[PalIdx].rgbRed;
                Green = Palette[PalIdx].rgbGreen;
                Blue = Palette[PalIdx].rgbBlue;

				// Write the ARGB data
				Data[X] = Blue | (Green << 8) | (Red << 16) | (Alpha << 24);
			}

		}
		pBuf->unlock();
	}

	/*-----------------------------------------------------------------*/
	/*------------------------- Ogre stuff ----------------------------*/
	/*-----------------------------------------------------------------*/
	int ManualFonFileLoader::CreateOgreFont(Font* DarkFont)
	{
		DarkFont->setSource(mTxtName);
		DarkFont->setType(FT_IMAGE);

		for(CharInfoList::const_iterator It = mChars.begin(); It != mChars.end(); It++)
		{
			const CharInfo& Char = *It;
			DarkFont->setGlyphTexCoords(Char.Code, (float)(Char.X - 2) / mImageDim,
				(float)(Char.Y - 2) / mImageDim, (float)(Char.X + Char.Width + 2) / mImageDim,
				(float)(Char.Y + mNumRows + 2) / mImageDim, 1.0);
		}

		DarkFont->setParameter("size", StringConverter::toString(mNumRows)); // seems mNumRows (NumRows) means Y size of the font...
		DarkFont->load();		//Let's rock!

		return 0;
	}

	//-------------------------------------------------------------------
    void ManualFonFileLoader::setPalette(PaletteType PalType, String PalFileName)
	{
    	mPaletteType = PalType;
		mPaletteFileName = PalFileName;
    }

	//-------------------------------------------------------------------
    void ManualFonFileLoader::loadResource(Resource* resource)
	{
        // Cast to font, and fill
        Font* DarkFont = static_cast<Font*>(resource);

        // Fill. Find the file to be loaded by the name, and load it
        String FontName = DarkFont->getName();
        mFontGroup = DarkFont->getGroup();

        // Get the real filename from the nameValuePairList
        // That means: truncate to the last dot, append .fon to the filename
        size_t DotPos = FontName.find_last_of(".");
        String BaseName = FontName;
        
        if (DotPos != String::npos)
            BaseName = FontName.substr(0, DotPos);
		
		if(mPaletteType != ePT_Default)
		{
			if ((mPaletteType == ePT_DefaultBook) || 
			   (mPaletteFileName != "" && ((mPaletteType == ePT_PCX) || (mPaletteType == ePT_External))))  // Palette file specified
			{
				if (mPaletteType == ePT_DefaultBook)
				{
					// Find the accompanying BOOK.PCX
					size_t SlashPos = FontName.find_last_of("/");
					mPaletteFileName = "";
					if (SlashPos != String::npos)
						mPaletteFileName = BaseName.substr(0, SlashPos + 1);	//Include the slash
					mPaletteFileName += "BOOK.PCX";
				}
				LogManager::getSingleton().logMessage("Font DEBUG: Will load a custom file palette");
			} 
			else 
			{
				LogManager::getSingleton().logMessage("Non-Default palette type, but no filename specified (Defaulting to LG's default pal). Font name " + BaseName);
			}
		}

		mTxtName = BaseName + "_Txt";

        BaseName += ".fon";

        //Open the file
        Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(BaseName, mFontGroup, true, resource);
		mFontFile = FilePtr(new OgreFile(Stream));

        if(LoadDarkFont())
			LogManager::getSingleton().logMessage("An error occurred while loading the font " + BaseName);

		if(CreateOgreFont(DarkFont))
			LogManager::getSingleton().logMessage("An error occurred creating Ogre font");
    }    
}
