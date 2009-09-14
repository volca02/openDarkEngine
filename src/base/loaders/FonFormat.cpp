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
 *
 *  $Id$
 *
 *****************************************************************************/

#include "FonFormat.h"
#include "File.h"

namespace Opde {
	File& operator<<(File& st, const DarkFontHeader& h) {
		st << h.Format << h.Unknown << h.Palette;
		st.write(&h.Zeros1, sizeof(h.Zeros1));
		st << h.FirstChar << h.LastChar;
		st.write(&h.Zeros2, sizeof(h.Zeros2));
		st << h.WidthOffset << h.BitmapOffset << h.RowWidth << h.NumRows;
		
		return st;
	}
	
	File& operator>>(File& st, DarkFontHeader& h) {
		st >> h.Format >> h.Unknown >> h.Palette;
		st.read(&h.Zeros1, sizeof(h.Zeros1));
		st >> h.FirstChar >> h.LastChar;
		st.read(&h.Zeros2, sizeof(h.Zeros2));
		st >> h.WidthOffset >> h.BitmapOffset >> h.RowWidth >> h.NumRows;
		
		return st;
	}
	
	File& operator<<(File& st, const ExternalPaletteHeader& h) {
		st << h.RiffSig << h.RiffLength << h.PSig1 << h.PSig2 << h.Length;
		
		return st;
	}
	
	File& operator>>(File& st, ExternalPaletteHeader& h) {
		st >> h.RiffSig >> h.RiffLength >> h.PSig1 >> h.PSig2 >> h.Length;
		
		return st;
	}
	
	File& operator<<(File& st, const BITMAPFILEHEADER& h) {
		st << h.bfType << h.bfSize << h.bfReserved1 << h.bfReserved2 << h.bfOffBits;
		
		return st;
	}
	
	File& operator>>(File& st, BITMAPFILEHEADER& h) {
		st >> h.bfType >> h.bfSize >> h.bfReserved1 >> h.bfReserved2 >> h.bfOffBits;
		
		return st;
	}
	
	File& operator<<(File& st, const BITMAPINFOHEADER& h) {
		st << h.biSize << h.biWidth << h.biHeight << h.biPlanes << h.biBitCount << h.biCompression
			<< h.biSizeImage << h.biXPelsPerMeter << h.biYPelsPerMeter << h.biClrUsed << h.biClrImportant;
		
		return st;
	}
	
	File& operator>>(File& st, BITMAPINFOHEADER& h) {
		st >> h.biSize >> h.biWidth >> h.biHeight >> h.biPlanes >> h.biBitCount >> h.biCompression
			>> h.biSizeImage >> h.biXPelsPerMeter >> h.biYPelsPerMeter >> h.biClrUsed >> h.biClrImportant;
		
		return st;
	}
	
	File& operator<<(File& st, const RGBQuad& h) {
		st << h.rgbBlue << h.rgbGreen << h.rgbRed << h.rgbReserved;
		
		return st;
	}
	
	File& operator>>(File& st, RGBQuad& h) {
		st >> h.rgbBlue >> h.rgbGreen >> h.rgbRed >> h.rgbReserved;
		
		return st;
	}
	
	File& operator<<(File& st, const BITMAPINFO& h) {
		st << h.bmiHeader;
		
		return st;
	}
	
	File& operator>>(File& st, BITMAPINFO& h) {
		st >> h.bmiHeader;
		
		return st;
	}
	
	File& operator<<(File& st, const RGBTRIPLE& h) {
		st << h.rgbtBlue << h.rgbtGreen << h.rgbtRed;
		
		return st;
	}
	
	File& operator>>(File& st, RGBTRIPLE& h) {
		st >> h.rgbtBlue >> h.rgbtGreen >> h.rgbtRed;
		
		return st;
	}
	
}

