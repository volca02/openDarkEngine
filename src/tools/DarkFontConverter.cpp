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

#include "DarkFontConverter.h"

static char BLACK_INDEX =	0;
static char WHITE_INDEX =	1;


char** ReadFont(FILE *FontFilePointer, int *ResultingWidth, int *ResultingHeight, int *ResultingColor, const char *FontName)
{
	struct DarkFontHeader FontHeader;
	unsigned short *Widths;
	char *BitmapData;
	unsigned int NumChars;
	struct CharInfo *Chars;
	char *ImageData;
	char **RowPointers;
	unsigned int ImageWidth, ImageHeight, FinalSize = 1;
	unsigned int X, Y;
	unsigned int N, I;
	char *Ptr;
	FILE *FilePointer;
	char FontDefData[100];
	unsigned long FileSize;

	fread(&FontHeader, sizeof(FontHeader), 1, FontFilePointer);
	NumChars = FontHeader.LastChar - FontHeader.FirstChar + 1;

	Widths = (unsigned short *)calloc(NumChars + 1, sizeof(unsigned short));
	if (!Widths)
		return NULL;
	fseek(FontFilePointer, FontHeader.WidthOffset, SEEK_SET);
	fread(Widths, (NumChars + 1) * sizeof(unsigned short), 1, FontFilePointer);

	fseek(FontFilePointer, 0L, SEEK_END);
	FileSize = ftell(FontFilePointer);
	BitmapData = (char *)calloc(FileSize + 1, sizeof(char));
	if (!BitmapData)
	{
		free(Widths);
		return NULL;
	}
	fseek(FontFilePointer, FontHeader.BitmapOffset, SEEK_SET);
	fread(BitmapData, FileSize - FontHeader.BitmapOffset, 1, FontFilePointer);
	if (FontHeader.BitmapOffset < FontHeader.WidthOffset)
	{
		free(Widths);
		free(BitmapData);
		return NULL;
	}

	Chars = (struct CharInfo*)calloc(NumChars, sizeof(struct CharInfo));
	if (!Chars)
	{
		free(Widths);
		free(BitmapData);
		return NULL;
	}

	ImageHeight = ((NumChars / 16) + 2) * (FontHeader.NumRows + 4);
	
	Y = (FontHeader.NumRows / 2) + 2;
	X = 2;
	ImageWidth = 2;
	for (N = 0; N < NumChars; N++)
	{
		Chars[N].Code = FontHeader.FirstChar + N;
		Chars[N].Column = Widths[N];
		Chars[N].Width = Widths[N+1] - Widths[N];
		Chars[N].X = X;
		Chars[N].Y = Y;
		X += Chars[N].Width + 6;
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

	RowPointers = (char**)calloc(FinalSize, sizeof(char*));
	if (!RowPointers)
	{
		free(Chars);
		free(Widths);
		free(BitmapData);
		return NULL;
	}

	ImageData = (char*)malloc(FinalSize * FinalSize);
	if (!ImageData)
	{
		free(RowPointers);
		free(Chars);
		free(Widths);
		free(BitmapData);
		return NULL;
	}
	memset(ImageData, BLACK_INDEX, FinalSize * FinalSize);
	Ptr = ImageData;
	for (N = 0; N < FinalSize; N++)
	{
		RowPointers[N] = Ptr;
		Ptr += FinalSize;
	}

	sprintf(FontDefData,"%s.fontdef", FontName);
	FilePointer = fopen(FontDefData, "w");
	if (FilePointer == 0)
	{
		fprintf(stdout,"Can not create %s\n", FontDefData);
		return NULL;
	}
	sprintf(FontDefData,"%s\n{\n\n\ttype\timage\n\tsource\t%s.bmp\n\n", FontName, FontName);
	fwrite(FontDefData, strlen(FontDefData), 1, FilePointer);


	for (N = 0; N < NumChars ; N++)
	{
		X = Chars[N].X;
		Y = Chars[N].Y++;
		if(FontHeader.FirstChar + N == ' ')
			sprintf(FontDefData,"\t\t//glyph %c %f %f %f %f\n", FontHeader.FirstChar + N, (float)(Chars[N].X - 2) / FinalSize, (float)(Chars[N].Y - 2) / FinalSize, (float)(Chars[N].X + Chars[N].Width + 2) / FinalSize, (float)(Chars[N].Y + FontHeader.NumRows + 2) / FinalSize); //Ogre doesn't like the space character
		else
			sprintf(FontDefData,"\t\tglyph %c %f %f %f %f\n", FontHeader.FirstChar + N, (float)(Chars[N].X - 2) / FinalSize, (float)(Chars[N].Y - 2) / FinalSize, (float)(Chars[N].X + Chars[N].Width + 2) / FinalSize, (float)(Chars[N].Y + FontHeader.NumRows + 2) / FinalSize);
		fwrite(FontDefData, strlen(FontDefData), 1, FilePointer);
	}

	fwrite("\n\n}\n", 4, 1, FilePointer);
	fclose(FilePointer);

	if (FontHeader.Format == 0)
	{
		Ptr = BitmapData;
		for (I = 0; I < FontHeader.NumRows; I++)
		{
			for (N = 0; N < NumChars; N++)
			{
				Y = Chars[N].Column;
				for (X = 0; X < Chars[N].Width; Y++, X++)
					RowPointers[Chars[N].Y + I][Chars[N].X + X] = ((Ptr[Y / 8]>>(7 - (Y % 8))) & 1) ? WHITE_INDEX : BLACK_INDEX;
			}
			Ptr += FontHeader.RowWidth;
		}
	}
	else
	{
		Ptr = BitmapData;
		for (I = 0; I < FontHeader.NumRows; I++)
		{
			for (N = 0; N < NumChars; N++)
			{
				memcpy(RowPointers[Chars[N].Y + I]+Chars[N].X, Ptr, Chars[N].Width);				
				Ptr += Chars[N].Width;
			}
		}
	}
	free(Chars);
	fprintf(stdout, "Read %d glyphs (first char = 0x%02x)\n", NumChars, FontHeader.FirstChar);
	*ResultingWidth = *ResultingHeight = FinalSize;
	switch (FontHeader.Format)
	{
	case 0:
		*ResultingColor = 1; break;
	case 1:
		*ResultingColor = 2; break;
	case 0xCCCC:
		*ResultingColor = (FontHeader.Palette == 1) ? -1 : 0;
		break;
	default:
		fprintf(stdout, "Unknown pixel Format! (0x%04X) Assuming 8bpp.\n", FontHeader.Format);
		*ResultingColor = 0;
		break;
	}

	free(Widths);
	free(BitmapData);
	return RowPointers;
}


int WriteImage(FILE *FilePointer, RGBQUAD *ColorTable, char **RowPointers, int ImageWidth, int ImageHeight)
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

	if(!fwrite(&FileHeader, sizeof(BITMAPFILEHEADER), 1, FilePointer))
		return -1;
	if(!fwrite(&BitmapHeader, sizeof(BITMAPINFOHEADER), 1, FilePointer))
		return -1;
	if(!fwrite(ColorTable, sizeof(RGBQUAD) * 256, 1, FilePointer))
		return -1;
	RowWidth -= ImageWidth;
	for (Row = ImageHeight-1; Row >= 0; Row--)
	{
		if(!fwrite(RowPointers[Row], ImageWidth, 1, FilePointer))
			return -1;
		if (RowWidth != 0)
		{
			if(!fwrite(Zero, RowWidth, 1, FilePointer))
				return -1;
		}
	}
	return 0;
}


RGBQUAD* ReadPalette(FILE *FilePointer)
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
	fread(&PaletteHeader, 20, 1, FilePointer);
	if (PaletteHeader.RiffSig == 0x46464952)
	{
		if (PaletteHeader.PSig1 != 0x204C4150)
		{
			free(Palette);
			return NULL;
		}
		fseek(FilePointer, 2L, SEEK_CUR);
		fread(&Count, 2, 1, FilePointer);
		if (Count > 256)
			Count = 256;
		fread(Palette, sizeof(RGBQUAD) * Count, 1, FilePointer);
		for (I = 0; I < Count; I++)
		{
			S = Palette[I].rgbRed;
			Palette[I].rgbRed = Palette[I].rgbBlue;
			Palette[I].rgbBlue = S;
		}
	}
	else if (PaletteHeader.RiffSig == 0x4353414A)
	{
		rewind(FilePointer);
		Buffer = (char*)malloc(3360);
		if (!Buffer)
		{
			free(Palette);
			return NULL;
		}
		fread(Buffer, 3352, 1, FilePointer);
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
		rewind(FilePointer);
		P = (RGBTRIPLE*)calloc(256,sizeof(RGBTRIPLE));
		if (!P)
		{
			free(Palette);
			return NULL;
		}
		Bytes = fread(P, sizeof(RGBTRIPLE) * 256, 1, FilePointer) * sizeof(RGBTRIPLE) * 256;
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


int FontToImage(const char *FontName, const char *PaletteFile)
{
	FILE *FilePointer;
	COLORREF *PaletteData;
	char **ImageRows;
	int ImageWidth, ImageHeight;
	int Color;
	int Error;
	char FileName[30];

	if (PaletteFile)
	{
		FilePointer = fopen(PaletteFile, "r");
		if (FilePointer == 0)
		{
			fprintf(stdout, "\n%s could not be opened.\n", PaletteFile);
			return -1;
		}
		PaletteData = (COLORREF*)ReadPalette(FilePointer);
		fclose(FilePointer);
		if (!PaletteData)
		{
			return 2;
		}
	}
	else
		PaletteData = (COLORREF*)ColorTable;
	sprintf(FileName,"%s.fon", FontName);
	FilePointer = fopen(FileName, "rb");
	if (FilePointer == 0)
	{
		fprintf(stdout, "\n%s could not be opened.\n", FileName);
		if (PaletteData != ColorTable)
			free(PaletteData);
		return -1;
	}
	ImageRows = ReadFont(FilePointer, &ImageWidth, &ImageHeight, &Color, FontName);
	fclose(FilePointer);
	if (!ImageRows)
	{
		if (PaletteData != ColorTable)
			free(PaletteData);
		return 2;
	}
	sprintf(FileName,"%s.bmp", FontName);
	FilePointer = fopen(FileName, "wb");
	if (FilePointer == 0)
	{
		free(*ImageRows);
		free(ImageRows);
		if (PaletteData != ColorTable)
			free(PaletteData);
		return -1;
	}
	if (Color == 2 && PaletteData == ColorTable)
		PaletteData = (COLORREF*)AntiAliasedColorTable;
	Error = WriteImage(FilePointer, (RGBQUAD*)PaletteData, ImageRows, ImageWidth, ImageHeight);
	fclose(FilePointer);
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

	if (argc < 2)
	{
		fprintf(stdout, "Copyright (C) 2005-2007 openDarkEngine team\n");
    	fprintf(stdout, "Based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>\n\n");
		fprintf(stdout, "Usage: DarkFontConverter FontFileName [PaletteFile]\n");
		return 1;
	}
	Font = argv[1];
	Palette = (argc > 1) ? argv[2] : NULL;
	Error = FontToImage(Font, Palette);

	return Error;
}

