/******************************************************************************
 *    DarkFontConverter.cpp
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *    Based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>
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
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "DarkFontConverter.h"

static char BLACK_INDEX =	0;
static char WHITE_INDEX =	1;


char** ReadFont(char *FontData, int *ResultingWidth, int *ResultingHeight, int *ResultingColor, const char *FontName)
{
	struct DarkFontHeader *FontHeader;
	unsigned short *Widths;
	char *BitmapData;
	unsigned int NumChars;
	struct CharInfo *Chars;
	char *ImageData;
	char **RowPointers;
	unsigned int ImageWidth, ImageHeight;
	unsigned int X, Y;
	unsigned int N, I;
	char *Ptr;
	HANDLE hFile;
	DWORD dwBytes;
	char FontDefData[100];

	FontHeader = (struct DarkFontHeader*)FontData;
	Widths = (unsigned short*)(FontData + FontHeader->WidthOffset);
	BitmapData = FontData + FontHeader->BitmapOffset;
	if (FontHeader->BitmapOffset < FontHeader->WidthOffset)
		return NULL;
	NumChars = FontHeader->LastChar - FontHeader->FirstChar + 1;

	Chars = (struct CharInfo*)calloc(NumChars, sizeof(struct CharInfo));
	if (!Chars)
		return NULL;

	ImageHeight = ((NumChars / 16) + 2) * (FontHeader->NumRows + 4);
	RowPointers = (char**)calloc(ImageHeight, sizeof(char*));
	if (!RowPointers)
	{
		free(Chars);
		return NULL;
	}
	Y = (FontHeader->NumRows / 2) + 2;
	X = 2;
	ImageWidth = 2;
	for (N = 0; N < NumChars; N++)
	{
		Chars[N].Code = FontHeader->FirstChar + N;
		Chars[N].Column = Widths[N];
		Chars[N].Width = Widths[N+1] - Widths[N];
		Chars[N].X = X;
		Chars[N].Y = Y;
		X += Chars[N].Width + 6;
		if ((N & 0xF) == 0xF)
		{
			Y += FontHeader->NumRows + 4;
			if (X > ImageWidth)
				ImageWidth = X;
			X = 2;
		}
	}
	if (X > ImageWidth)
		ImageWidth = X;

	ImageData = (char*)malloc(ImageHeight * ImageWidth);
	if (!ImageData)
	{
		free(RowPointers);
		free(Chars);
		return NULL;
	}
	memset(ImageData, BLACK_INDEX, ImageHeight * ImageWidth);
	Ptr = ImageData;
	for (N = 0; N < ImageHeight; N++)
	{
		RowPointers[N] = Ptr;
		Ptr += ImageWidth;
	}

	sprintf(FontDefData,"%s.fontdef", FontName);
	hFile = CreateFile(FontDefData, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	sprintf(FontDefData,"%s\n{\n\n\ttype\timage\n\tsource\t%s.bmp\n\n", FontName, FontName);
	WriteFile(hFile, FontDefData, strlen(FontDefData), &dwBytes, NULL);


	for (N = 0; N < NumChars ; N++)
	{
		X = Chars[N].X;
		Y = Chars[N].Y++;
		sprintf(FontDefData,"\t\tglyph %c %f %f %f %f\n", FontHeader->FirstChar + N, (float)(Chars[N].X - 2) / ImageWidth, (float)(Chars[N].Y - 2) / ImageHeight, (float)(Chars[N].X + Chars[N].Width + 2) / ImageWidth, (float)(Chars[N].Y + FontHeader->NumRows + 2) / ImageHeight);
		WriteFile(hFile, FontDefData, strlen(FontDefData), &dwBytes, NULL);
	}

	WriteFile(hFile, "\n\n}\n", 4, &dwBytes, NULL);
	CloseHandle(hFile);

	if (FontHeader->Format == 0)
	{
		Ptr = BitmapData;
		for (I = 0; I < FontHeader->NumRows; I++)
		{
			for (N = 0; N < NumChars; N++)
			{
				Y = Chars[N].Column;
				for (X = 0; X < Chars[N].Width; Y++, X++)
					RowPointers[Chars[N].Y + I][Chars[N].X + X] = ((Ptr[Y / 8]>>(7 - (Y % 8))) & 1) ? WHITE_INDEX : BLACK_INDEX;
			}
			Ptr += FontHeader->RowWidth;
		}
	}
	else
	{
		Ptr = BitmapData;
		for (I = 0; I < FontHeader->NumRows; I++)
		{
			for (N = 0; N < NumChars; N++)
			{
				memcpy(RowPointers[Chars[N].Y + I]+Chars[N].X, Ptr, Chars[N].Width);
				Ptr += Chars[N].Width;
			}
		}
	}
	free(Chars);
	fprintf(stdout, "Read %d glyphs (first char = 0x%02x)\n", NumChars, FontHeader->FirstChar);
	*ResultingWidth = ImageWidth;
	*ResultingHeight = ImageHeight;
	switch (FontHeader->Format)
	{
	case 0:
		*ResultingColor = 1; break;
	case 1:
		*ResultingColor = 2; break;
	case 0xCCCC:
		*ResultingColor = (FontHeader->Palette == 1) ? -1 : 0;
		break;
	default:
		fprintf(stdout, "Unknown pixel Format! (0x%04X) Assuming 8bpp.\n", FontHeader->Format);
		*ResultingColor = 0;
		break;
	}
	return RowPointers;
}


DWORD WriteImage(HANDLE hImageFile, RGBQUAD *ColorTable, char **RowPointers, int ImageWidth, int ImageHeight)
{
	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	BitmapHeader;
	int 	RowWidth, Row;
	DWORD	dwBytes;
	char	Zero[4] = {0,0,0,0};

	FileHeader.bfType = 0x4D42;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;
	FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*256);
	BitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapHeader.biWidth = ImageWidth;
	BitmapHeader.biHeight = ImageHeight;
	BitmapHeader.biPlanes = 1;
	BitmapHeader.biBitCount = 8;
	BitmapHeader.biCompression = 0;
	BitmapHeader.biXPelsPerMeter = 0xB13;
	BitmapHeader.biYPelsPerMeter = 0xB13;
	BitmapHeader.biClrUsed = 256;
	BitmapHeader.biClrImportant = 256;
	RowWidth = (ImageWidth + 3) & ~3;
	BitmapHeader.biSizeImage = RowWidth * ImageHeight;
	FileHeader.bfSize = FileHeader.bfOffBits + BitmapHeader.biSizeImage;

	if (!WriteFile(hImageFile, &FileHeader, sizeof(BITMAPFILEHEADER), &dwBytes, NULL))
		return GetLastError();
	if (!WriteFile(hImageFile, &BitmapHeader, sizeof(BITMAPINFOHEADER), &dwBytes, NULL))
		return GetLastError();
	if (!WriteFile(hImageFile, ColorTable, sizeof(RGBQUAD)*256, &dwBytes, NULL))
		return GetLastError();
	RowWidth -= ImageWidth;
	for (Row = ImageHeight-1; Row >= 0; Row--)
	{
		if (!WriteFile(hImageFile, RowPointers[Row], ImageWidth, &dwBytes, NULL))
			return GetLastError();
		if (RowWidth != 0)
		{
			if (!WriteFile(hImageFile, Zero, RowWidth, &dwBytes, NULL))
				return GetLastError();
		}
	}
	return 0;
}


RGBQUAD* ReadPalette(HANDLE hPalFile)
{
	struct
	{
		DWORD RiffSig;
		DWORD RiffLength;
		DWORD PSig1;
		DWORD PSig2;
		DWORD Length;
	} PaletteHeader;
	RGBQUAD *Palette;
	RGBTRIPLE *P;
	DWORD Bytes;
	WORD Count;
	char *Buffer, *C;
	BYTE S;
	int I;

	Palette = (RGBQUAD*)calloc(256, sizeof(RGBQUAD));
	if (!Palette)
		return NULL;
	ReadFile(hPalFile, &PaletteHeader, 20, &Bytes, NULL);
	if (PaletteHeader.RiffSig == 0x46464952)
	{
		if (PaletteHeader.PSig1 != 0x204C4150)
		{
			free(Palette);
			return NULL;
		}
		SetFilePointer(hPalFile, 2, NULL, FILE_CURRENT);
		ReadFile(hPalFile, &Count, 2, &Bytes, NULL);
		if (Count > 256)
			Count = 256;
		ReadFile(hPalFile, Palette, sizeof(RGBQUAD)*Count, &Bytes, NULL);
		for (I = 0; I < Count; I++)
		{
			S = Palette[I].rgbRed;
			Palette[I].rgbRed = Palette[I].rgbBlue;
			Palette[I].rgbBlue = S;
		}
	}
	else if (PaletteHeader.RiffSig == 0x4353414A)
	{
		SetFilePointer(hPalFile, 0, 0, FILE_BEGIN);
		Buffer = (char*)malloc(3360);
		if (!Buffer)
		{
			free(Palette);
			return NULL;
		}
		ReadFile(hPalFile, Buffer, 3352, &Bytes, NULL);
		if (strncmp(Buffer, "JASC-PAL", 8))
		{
			free(Buffer);
			free(Palette);
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
		free(Buffer);
	}
	else
	{
		SetFilePointer(hPalFile, 0, 0, FILE_BEGIN);
		P = (RGBTRIPLE*)calloc(256,sizeof(RGBTRIPLE));
		if (!P)
		{
			free(Palette);
			return NULL;
		}
		ReadFile(hPalFile, P, sizeof(RGBTRIPLE)*256, &Bytes, NULL);
		Count = (WORD)Bytes / sizeof(RGBTRIPLE);
		for (I = 0; I < Count; I++)
		{
			Palette[I].rgbRed = P[I].rgbtBlue;
			Palette[I].rgbGreen = P[I].rgbtGreen;
			Palette[I].rgbBlue = P[I].rgbtRed;
		}
		free(P);
	}
	return Palette;
}


char* loadfile(HANDLE hFile)
{
	HANDLE hFileMap;
	char *pRet;

	hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!hFileMap)
		return NULL;
	pRet = (char *)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	CloseHandle(hFileMap);
	return pRet;
}


int fon2image(const char *FontName, const char *PaletteFile)
{
	HANDLE hFile;
	COLORREF *PaletteData;
	char **ImageRows;
	int ImageWidth, ImageHeight;
	char *FontDefData;
	int Color;
	int Error;
	char FileName[30];

	if (PaletteFile)
	{
		hFile = CreateFile(PaletteFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			fprintf(stdout, "\n%s could not be opened.\n", PaletteFile);
			return GetLastError();
		}
		PaletteData = (COLORREF*)ReadPalette(hFile);
		CloseHandle(hFile);
		if (!PaletteData)
		{
			return 2;
		}
	}
	else
		PaletteData = (COLORREF*)ColorTable;
	sprintf(FileName,"%s.fon", FontName);
	hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "\n%s could not be opened.\n", FileName);
		if (PaletteData != ColorTable)
			free(PaletteData);
		return GetLastError();
	}
	FontDefData = loadfile(hFile);
	Error = GetLastError();
	if (!FontDefData)
	{
		CloseHandle(hFile);
		if (PaletteData != ColorTable)
			free(PaletteData);
		return Error;
	}
	ImageRows = ReadFont(FontDefData, &ImageWidth, &ImageHeight, &Color, FontName);
	UnmapViewOfFile(FontDefData);
	CloseHandle(hFile);
	if (!ImageRows)
	{
		if (PaletteData != ColorTable)
			free(PaletteData);
		return 2;
	}
	sprintf(FileName,"%s.bmp", FontName);
	hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		free(*ImageRows);
		free(ImageRows);
		if (PaletteData != ColorTable)
			free(PaletteData);
		return GetLastError();
	}
	if (Color == 2 && PaletteData == ColorTable)
		PaletteData = (COLORREF*)AntiAliasedColorTable;
	Error = WriteImage(hFile, (RGBQUAD*)PaletteData, ImageRows, ImageWidth, ImageHeight);
	CloseHandle(hFile);
	free(*ImageRows);
	free(ImageRows);
	if (PaletteData != ColorTable && PaletteData != AntiAliasedColorTable)
		free(PaletteData);
	return Error;
}


int main(int argc, char **argv)
{
	char *Font, *Palette;
	int Error = 0;

	fprintf(stdout, "Dark Font Converter\n");

	if (argc < 1)
	{
		fprintf(stdout, "Copyright (C) 2005-2007 openDarkEngine team\n");
    	fprintf(stdout, "Based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>\n\n");
		fprintf(stdout, "Usage: DarkFontConverter FontFileName [PaletteFile]\n");
		return 1;
	}
	Font = argv[1];
	Palette = (argc > 1) ? argv[2] : NULL;
	Error = fon2image(Font, Palette);

	return Error;
}

