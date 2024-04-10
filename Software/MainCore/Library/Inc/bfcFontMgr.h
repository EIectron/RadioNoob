#ifndef BFC_FONT_MGR_H
#define BFC_FONT_MGR_H

#include "bfcfont.h"

//  bpp: 1, 2, 4 or 8 (bits per pixel)
int   GetFontBpp(unsigned long FontType);
//  bit order: 0 (Big Endian), or 1 (Little Endian)
int   GetFontEndian(unsigned long FontType);
//  scan based: 0 (row), or 1 (column)
int   GetFontScanBase(unsigned long FontType);
//  scan prfered: 0 (row), or 1 (column)
int   GetFontScanPrefer(unsigned long FontType);
//  data packed: 0 (No), or 1 (Yes)
int   GetFontDataPack(unsigned long FontType);
//	return font height in pixels
int   GetFontHeight(const BFC_FONT *pFont);
//	get structure BFC_CHARINFO pointer
const BFC_CHARINFO* GetCharInfo(const BFC_FONT *pFont, unsigned short ch);

#endif	//BFC_FONT_MGR_H