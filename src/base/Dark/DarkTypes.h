/******************************************************************************
 *    DarkTypes.h
 *
 *    This file is part of DarkUtils
 *    Copyright (C) 2004 Tom N Harris <telliamed@whoopdedo.cjb.net>
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
 *	  $Id$
 *
 *****************************************************************************/

/***********************************************************
 * Custom data types used for the Dark database
 * (and some convenience shortcuts because portability is a bitch)
 */
#ifndef DARK_DARKDBTYPES_H
#define DARK_DARKDBTYPES_H

#include "utils.h"

#ifdef __cplusplus
namespace Dark {
#endif

/* LGS called this 'mxs_vector' */
struct DarkCoord {
    float x, y, z;
};
typedef struct DarkCoord Coord;

/* LGS called this 'mxs_angvec */
struct DarkCoordShort // used for rotation
{
    uint16 x, y, z;
};
typedef struct DarkCoordShort SCoord;

struct Dark2Coord {
    float x, y;
};
typedef struct Dark2Coord XYCoord;

struct DarkColor {
    float r, g, b;
};
typedef struct DarkColor Color;

struct DarkPlane {
    Coord normal;
    float v;
};
typedef struct DarkPlane Plane;

/* 15-bit color, Used in WRRGB */
typedef unsigned short DarkBGR;
inline uint8 DarkRED(DarkBGR c) { return (uint8)(c & 0x1F); }
inline uint8 DarkGREEN(DarkBGR c) { return (uint8)((c & 0x03E0) >> 5); }
inline uint8 DarkBLUE(DarkBGR c) { return (uint8)((c & 0x7C00) >> 10); }
inline DarkBGR BGR(uint8 b, uint8 g, uint8 r) {
    return (DarkBGR)((((uint16)b) << 10) | (((uint16)g) << 5) | ((uint16)r));
}

/* Generic length + variable-sized string */
#pragma pack(push, 1)
struct DarkString {
    uint32 length;
    char name[1];
};
#pragma pack(pop)

/* Generic true/false value */
#define DarkFalse 0
#define DarkTrue 1
struct DarkBoolean {
    Boolean p;
};

/* Fixed-length text. */
struct DarkLabel {
    char name[16];
};

/* Pseudo-scripts */
struct DarkPScriptCommand {
    uint32 action;
    char arg[3][64];
    char zero[64];
};
#define PSCRIPT_NOTHING 0
#define PSCRIPT_SCRMESSAGE 1
#define PSCRIPT_PLAY 2
#define PSCRIPT_ALERT 3
#define PSCRIPT_BEHOSTILE 4
#define PSCRIPT_INVESTIGATE 5
#define PSCRIPT_GOTO 6
#define PSCRIPT_FROB 7
#define PSCRIPT_WAIT 8
#define PSCRIPT_MPRINT 9
#define PSCRIPT_METAPROP 10
#define PSCRIPT_ADDLINK 11
#define PSCRIPT_REMLINK 12
#define PSCRIPT_FACE 13
#define PSCRIPT_SIGNAL 14
#define PSCRIPT_DESTSCRMESSAGE 15

#define PRIORITY_NONE 0
#define PRIORITY_VERYLOW 1
#define PRIORITY_LOW 2
#define PRIORITY_NORMAL 3
#define PRIORITY_HIGH 4
#define PRIORITY_VERYHIGH 5
#define PRIORITY_ABSOLUTE 6

#define MESHJOINT_NONE 0
#define MESHJOINT_HEAD 1
#define MESHJOINT_NECK 2
#define MESHJOINT_ABDOMEN 3
#define MESHJOINT_BUTT 4
#define MESHJOINT_LSHOULDER 5
#define MESHJOINT_RSHOULDER 6
#define MESHJOINT_LELBOW 7
#define MESHJOINT_RELBOW 8
#define MESHJOINT_LWRIST 9
#define MESHJOINT_RWRIST 10
#define MESHJOINT_LFINGERS 11
#define MESHJOINT_RFINGERS 12
#define MESHJOINT_LHIP 13
#define MESHJOINT_RHIP 14
#define MESHJOINT_LKNEE 15
#define MESHJOINT_RKNEE 16
#define MESHJOINT_LANKLE 17
#define MESHJOINT_RANKLE 18
#define MESHJOINT_LTOE 19
#define MESHJOINT_RTOE 20
#define MESHJOINT_TAIL 21

#ifdef __cplusplus
} // namespace Dark
#endif

#endif
