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

//---------------------------- El cargador de las fuentes para El Motor Oscuro --------------------
#include "FreeImage.h"
#include "ManualFonFileLoader.h"
#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>

using namespace std;
using namespace Opde; //For the Opde::File

// Both horizontal and vertical spacing for characters
#define _SPACING 2

namespace Ogre 
{
    /*-----------------------------------------------------------------*/
	/*--------------------- ManualFonFileLoader -----------------------*/
	/*-----------------------------------------------------------------*/
    ManualFonFileLoader::ManualFonFileLoader() : ManualResourceLoader(), mChars()
	{
		mpMemBuff = NULL;
    }

    //-------------------------------------------------------------------
    ManualFonFileLoader::~ManualFonFileLoader() 
	{
		delete [] mpMemBuff;
    }


	/*-----------------------------------------------------------------*/
	/*------------------------ Bitmap stuff ---------------------------*/
	/*-----------------------------------------------------------------*/
	unsigned char** ManualFonFileLoader::ReadFont(FilePtr FontFile, int *ResultingColor)
	{
		struct DarkFontHeader FontHeader;
		unsigned int ImageWidth, ImageHeight, FinalSize = 1;
		unsigned int X, Y;
		unsigned int N;
		unsigned char *Ptr;
		unsigned int NumChars;

		mChars.clear();

		FontFile->readStruct(&FontHeader, DarkFontHeader_Format, sizeof(DarkFontHeader));
		NumChars = FontHeader.LastChar - FontHeader.FirstChar + 1;
		mNumRows = FontHeader.NumRows;

		if (NumChars < 0)
			return NULL;

		vector <unsigned short> Widths;
		FontFile->seek(FontHeader.WidthOffset, File::FSEEK_BEG);
		for(N = 0; N < NumChars + 1; N++)
		{
			unsigned short Temp;
			FontFile->readElem(&Temp, sizeof(Temp));
			Widths.push_back(Temp);
		}

		if (FontHeader.BitmapOffset < FontHeader.WidthOffset)
			return NULL;

		unsigned char *BitmapData = new unsigned char[FontFile->size() + 1];
		
		if (!BitmapData)
			return NULL;
		

		FontFile->seek(FontHeader.BitmapOffset, File::FSEEK_BEG);
		FontFile->read(BitmapData, FontFile->size() - FontHeader.BitmapOffset);		
		
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
	RGBQUAD* ManualFonFileLoader::ReadPalette(StdFile *FilePointer)
	{
		ExternalPaletteHeader PaletteHeader;
		WORD Count;
		char *Buffer, *C;
		BYTE S;
		unsigned int I;

		RGBQUAD *Palette = new RGBQUAD[256];
		if (!Palette)
			return NULL;
		FilePointer->readStruct(&PaletteHeader, ExternalPaletteHeader_Format, sizeof(ExternalPaletteHeader));
		if (PaletteHeader.RiffSig == 0x46464952)
		{
			if (PaletteHeader.PSig1 != 0x204C4150)
			{
				delete []Palette;
				return NULL;
			}
			FilePointer->seek(2L, StdFile::FSEEK_CUR);
			FilePointer->readElem(&Count, 2);
			if (Count > 256)
				Count = 256;
			for (I = 0; I < Count; I++)
			{
				FilePointer->readStruct(&Palette[I], RGBQUAD_Format, sizeof(RGBQUAD));
				S = Palette[I].rgbRed;
				Palette[I].rgbRed = Palette[I].rgbBlue;
				Palette[I].rgbBlue = S;
			}
		}
		else if (PaletteHeader.RiffSig == 0x4353414A)
		{
			FilePointer->seek(0);
			Buffer = new char[3360];
			if (!Buffer)
			{
				delete []Palette;
				return NULL;
			}
			FilePointer->read(Buffer, 3352);
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
			FilePointer->seek(0);
			RGBTRIPLE P;
			for (I = 0; I < FilePointer->size() / sizeof(RGBTRIPLE); I++)
			{
				FilePointer->readStruct(&P, RGBTRIPLE_Format, sizeof(RGBTRIPLE));
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

		mBmpFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256) + (mImageDim * mImageDim);
		mpMemBuff = new unsigned char[mBmpFileSize];
		if(!mpMemBuff)
			return -1;
		unsigned char *Ptr = mpMemBuff;

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
		BitmapFile->write(mpMemBuff, mBmpFileSize);
		delete BitmapFile;*/

		return 0;
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::LoadDarkFont(FilePtr FontFile, FilePtr BookFile, bool HasBook)
	{
		COLORREF *PaletteData;
		unsigned char **ImageRows;
		int Color;

		/*if (PaletteFileName != "")
		{
			// Small comment: This would better be done using Ogre::DataStream
			// Also, the StdFile (as others) will throw an exception when the file was not found,
			// so the condition will always fail...
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
		}*/
		if(HasBook)	//TODO: Check how NULL can be tested on BookFile directly
		{
			RGBQUAD *Palette = new RGBQUAD[256];
			BookFile->seek(3 * 256 + 1, File::FSEEK_END);
			BYTE Padding;
			BookFile->readElem(&Padding, 1);
			if(Padding == 0x0C)
			{
				for (unsigned int I = 0; I < 256; I++)
				{
					BookFile->readElem(&Palette[I].rgbRed, 1);		//TODO: Endianess
					BookFile->readElem(&Palette[I].rgbGreen, 1);
					BookFile->readElem(&Palette[I].rgbBlue, 1);
					Palette[I].rgbReserved = 0;
				}
				PaletteData = (COLORREF*)Palette;
			}			
			else
				PaletteData = (COLORREF*)ColorTable;	//Not an 8bpp or invalid PCX, use the standard palette
		}
		else
			PaletteData = (COLORREF*)ColorTable;

		ImageRows = ReadFont(FontFile, &Color);
		if (!ImageRows)
		{
			if (PaletteData != ColorTable)
				delete []PaletteData;
			return 2;
		}
		if (Color == 2 && PaletteData == ColorTable)
			PaletteData = (COLORREF*)AntiAliasedColorTable;

		// WriteImage((RGBQUAD*)PaletteData, ImageRows);
		// For now... Can also be done directly when loading (And probably better too)
		createOgreTexture(ImageRows, (RGBQUAD*)PaletteData);


		delete [] ImageRows;
		if (PaletteData != ColorTable && PaletteData != AntiAliasedColorTable)
			delete []PaletteData;
		return 0;
	}

    //-------------------------------------------------------------------
	void ManualFonFileLoader::createOgreTexture(unsigned char** img, RGBQUAD* palette) {
		// Create a texure, then fill it
		TexturePtr txt = TextureManager::getSingleton().createManual(mTxtName, mFontGroup, TEX_TYPE_2D, mImageDim, mImageDim, 1, PF_A8R8G8B8);

		// Lock the texture, obtain a pointer
		HardwarePixelBufferSharedPtr pbuf = txt->getBuffer(0, 0);

		// Erase the lmap atlas pixel buffer
		pbuf->lock(HardwareBuffer::HBL_DISCARD);
		const PixelBox &pb = pbuf->getCurrentLock();

		// Copy the image data, converting to 32bit on the fly...
		for (unsigned int y = 0; y < mImageDim; y++) {
		    unsigned char* row = img[y];
			uint32 *data = static_cast<uint32*>(pb.data) + y*pb.rowPitch;

			for (unsigned int x = 0; x < mImageDim; x++) {
				int palidx = row[x];

				unsigned char r,g,b,a;

                a = 255;
				if (palidx == BLACK_INDEX)
                    a = 0;

				palidx = 255 - palidx;

                r = palette[palidx].rgbRed;
                g = palette[palidx].rgbGreen;
                b = palette[palidx].rgbBlue;

				// Write the ARGB data
				data[x] = b | (g << 8) | (r << 16) | (a << 24);
			}

		}

		pbuf->unlock();
	}

	/*-----------------------------------------------------------------*/
	/*------------------------- Alpha stuff ---------------------------*/
	/*-----------------------------------------------------------------*/
	int ManualFonFileLoader::AddAlpha()
	{
#ifdef FREEIMAGE_LIB
		FreeImage_Initialise();
#endif //FREEIMAGE_LIB

		FIMEMORY *BmpMem = FreeImage_OpenMemory(mpMemBuff, mBmpFileSize);

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
		delete [] mpMemBuff;
		mpMemBuff = NULL;

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
        mFontGroup = DarkFont->getGroup();

        // Get the real filename from the nameValuePairList
        // That means: truncate to the last dot, append .fon to the filename
        size_t DotPos = FontName.find_last_of(".");
        String BaseName = FontName;
        if (DotPos != String::npos)
            BaseName = FontName.substr(0, DotPos);
		
		size_t SlashPos = FontName.find_last_of("/");
        String BookName = "";
        if (SlashPos != String::npos)
            BookName = BaseName.substr(0, SlashPos + 1);	//Include the slash
		BookName += "BOOK.PCX";


		mTxtName = BaseName + "_Txt";

        BaseName += ".fon";

        //Open the file
        Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(BaseName, mFontGroup, true, resource);
		FilePtr FontFile = new OgreFile(Stream);

		FilePtr BookFile;
		bool HasBook = FALSE;
		if(Ogre::ResourceGroupManager::getSingleton().resourceExists (mFontGroup, BookName))
		{
			Ogre::DataStreamPtr BStream = Ogre::ResourceGroupManager::getSingleton().openResource(BookName, mFontGroup, true, resource);
			BookFile = new OgreFile(BStream);
			HasBook = TRUE;
		}

        if(LoadDarkFont(FontFile, BookFile, HasBook))
			LogManager::getSingleton().logMessage("An error occurred while loading the font " + BaseName);
		/*
		if(AddAlpha())
			LogManager::getSingleton().logMessage("An error occurred while adding Alpha Channel");*/

		if(CreateOgreFont(DarkFont))
			LogManager::getSingleton().logMessage("An error occurred creating Ogre font");
    }
}
