/******************************************************************************
 *    DarkDBDefs.h
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
 * Structured data in the Dark database
 * Mostly chank data.
 *
 * "ChunkName" [file types] [games]
 * 1 - Thief 1/G  2 - Thief 2  S - SShock2  C - SShock2 Campaign
 * * - All games, or all file types (other than SShock2 Campaign)
 * Cows are generally a combination of GAM and MIS chunks.
 */
#ifndef DARK_DARKDBDEFS_H
#define DARK_DARKDBDEFS_H

#include "DarkTypes.h"

#ifdef __cplusplus
namespace Dark
{
#endif

/*
 * Header for Dark-engine Database.
 */
struct DarkDBHeader
{
	uint32	inv_offset;		// Offset to inventory TOC from top of header
	uint32	zero;
	uint32	one;
	uint8	zeros[256];
	uint32	dead_beef;		// 0xEFBEADDE (damn little-endian)
};

/*
 * Item in chunk index.
 */
struct DarkDBInvItem
{
	char	name[12];
	uint32	offset;
	uint32	length;
};

/*
 * Chunk index.
 */
struct DarkDBInventory
{
	uint32	count;
	struct DarkDBInvItem	item[];
};

/*
 * Universal chunk header.
 */
struct DarkDBChunkHeader
{
	char	name[12];
	uint32	version_high;
	uint32	version_low;
	uint32	zero;
};


/*
 * Generic link.
 * Each L$ chunk is an array of these.
 * link id = flavor<<20 + concrete_p<<16 + number
 */
#pragma pack(push,2)
struct DarkDBChunkLink
{
	sint32	id;		// Link ID
	sint32	src;
	sint32	dest;
	uint16	flavor;	// Index from Relations chunk.
};
#pragma pack(pop)

/*
 * Generic link data.
 */
struct DarkDBChunkLinkData
{
	uint32	length;		// size of data for each link
	// Optional array of link ID and data
};

struct DarkDBLinkData
{
	sint32	id;
	uint8	data[];
};

/*
 * Generic header for properties (P$ chunks).
 */
struct DarkDBChunkPropertyHeader
{
	sint32	id;		// object ID
	uint32	length;	// size of data
};


/*
 * OBJ_MAP [VBR,MIS,SAV] [*]
 * HotRegions [MIS] [*]
 * MultiBrush [MIS] [*]
 * HotRegions and MultiBrush begin with a throw-away entry of id = 0x0100 
 * and the name of the chunk, and are padded with a shitload of 0xFF
 */
#pragma pack(push,1)
struct DarkDBChunkPaddedName
{
	sint32	id;
	uint32	name_length;
	char	name[];		// bloody variable-length data field
};
#pragma pack(pop)


/*
 * AbyssScr [SAV] [C]
 * BaconScr [SAV] [C]
 * GolfScr [SAV] [C]
 * HoggerScore [SAV] [C]
 * OWScores [SAV] [C]
 * RacerScr [SAV] [C]
 * StackerScr [SAV] [C]
 * SShock2 Mini-game scores.
 * Golf points are 1000 - #strokes.
 * Description may have new-lines in it.
 * Engine Structure: sMiniGameScores
 */
struct DarkDBChunkMiniGameScores
{
	struct 
	{
		sint32	points;
		char	desc[128];
	} scores[10];
};

/*
 * AICONVERSE [VBR,MIS,SAV] [*]
 * List of active conversations.
 * Conversations are always restarted from the beginning.
 */
struct DarkDBChunkAICONVERSE
{
	uint32	count;
	sint32	conversations[];
};

/*
 * AIACS [GAM] [2,S]
 * AI Acuity Set.
 * Engine Structure: sAIAcuitySets
 */
struct DarkDBChunkAIACS
{
	struct 
	{
		float	lighting;
		float	movement;
		float	exposure;
	} acuity[6];
};
#define AI_ACUITY_NORMAL		0
#define AI_ACUITY_PERIPHERAL	1
#define AI_ACUITY_OMNI			2
#define AI_ACUITY_LIGHTONLY		3
#define AI_ACUITY_MOVEONLY		4
#define AI_ACUITY_LOWLIGHT		5

/*
 * AICRTSZ [GAM] [2,S]
 * AI Creature Sizes.
 * Dromed only recognizes the first one.
 * Might be worth investigating if the other entries can be used.
 * Engine Structure: sAICreatureSizes
 */
struct DarkDBChunkAICRTSZ
{
	struct
	{
		float	width;
		float	height;
	} type[8];
};

/*
 * AIGPTHVAR [GAM] [2]
 * AI Gamesys Path option.
 * Engine Structure: sAIGamesysPathOptions
 */
struct DarkDBChunkAIGPTHVAR
{
	float	large_door_size;
};

/*
 * AIHearStat [GAM] [*]
 * AI Hearing Stats.
 * The first entry in each array isn't in dromed.
 * Engine Structure: sHearingStats
 */
struct DarkDBChunkAIHearStat
{
	float	multiply[6];
	sint32	add[6];
};
#define AISTAT_NULL		0
#define AISTAT_VERYLOW	1
#define AISTAT_LOW		2
#define AISTAT_NORMAL	3
#define AISTAT_HIGH		4
#define AISTAT_VERYHIGH	5

/*
 * AIPATHVAR [MIS,SAV] [2]
 * AI Path options.
 * Engine Structure: sAIPathOptions
 */
struct DarkDBChunkAIPATHVAR
{
	Boolean	pathable_water;
};

/*
 * AISNDTWK [GAM] [*]
 * AI Sound Tweaks.
 * Engine Structure: sAISoundTweaks
 */
struct DarkDBChunkAISNDTWK
{
	sint32	untyped_range;
	sint32	inform_range;
	sint32	minor_range;
	sint32	major_range;
	sint32	noncombat_range;
	sint32	combat_range;
};

/*
 * ALARMCOUNT [MIS,SAV] [S]
 * ????? The active alarm countdown timer, perhaps?
 * Engine Structure: sAlarmCount
 */
struct DarkDBChunkALARMCOUNT
{
	uint32	count;
	/* XXX: verify */
};

/*
 * AMAPANNO [*] [2]
 * Auto-map annotations.
 */
struct DarkDBChunkAMAPANNO
{
	uint32 count;
	struct 
	{
		sint32	x,y;
		sint32	page;
		char	text[128];
	} notes[];
};

/*
 * AMBIENT [MIS,SAV] [*]
 * ?????
 */
struct DarkDBChunkAMBIENT
{
	uint32	unknown;
};

/*
 * BASH [GAM] [*]
 * Bash Velocity Vars.
 * Engine Structure: sBashVars
 */
struct DarkDBChunkBASH
{
	float	threshold;
	float	coefficient;
};

/*
 * BINDTABLE [SAV] [C]
 * ?????
 * Saved in the campaign file.
 * Engine Structure: sBindTable
 */
struct DarkDBChunkBINDTABLE
{
	uint32	unknown[12];
};

/*
 * BRHEAD [MIS] [*]
 * Stores user names and editing time. (info_window)
 * Some other stuff I don't know about.
 * Is removed by misstrip.
 */
struct DarkDBChunkBRHEAD
{
	char	creator[16];
	char	name[16];
	uint8	zero[64];
	uint32	unknown1;	// 0x00
	uint32	grid_size;
	Boolean	show_grid;
	uint32	shading;	// enable=0x2, disable=0x01 (or anything other than 2)
	uint32	unknown5;	// 0x00
	Boolean	use_grid;
	uint32	time;		// cumulative editing time, milliseconds
};

/*
 * CELOBJVAR [MIS,SAV] [2]
 * T2 New Sky Celestial Object.
 * There are three of these chunks: CELOBJVAR1, CELOBJVAR2, CELOBJVAR3
 * Engine Structure: sMissionCelestialObj
 */
struct DarkDBChunkCELOBJVAR
{
	Boolean	enable;
	Boolean	fog;
	Boolean	isalpha;
	char	texture[256];
	float	alpha;
	float	distance;
	float	angular_size;
	float	latitude;
	float	longitude;
	float	rotation;
	Color	color;
};

/*
 * CLOUDOBJVAR [MIS,SAV] [2]
 * T2 New Sky Cloud Deck.
 * Engine Structure: sMissionCloudObj
 */
struct DarkDBChunkCLOUDOBJVAR
{
	Boolean	enable;
	Boolean	fog;
	Boolean	isalpha;
	char	texture[256];
	float	alpha;
	float	height;
	float	tile_size;
	sint32	tiles_per_side;
	sint32	subtiles;
	Coord	wind;
	Color	color;
	Color	east_color;
	sint32	east_method;
	float	east_scale;
	Color	west_color;
	sint32	west_method;
	float	west_scale;
	float	rotation;
	sint32	subtile_alpha;
	Color	glow_color;
	sint32	glow_method;
	float	glow_scale;
	float	glow_latitude;
	float	glow_longitude;
	float	glow_angle;
	sint32	glow_tiles;
};
#define COLOR_METHOD_SUM			0
#define COLOR_METHOD_INTERPOLATE	1

/*
 * DARKCOMBAT [GAM] [*]
 * Dark Combat Vars.
 * Engine Structure: sCombatVars
 */
struct DarkDBChunkDARKCOMBAT
{
	sint32	backstab_bonus;
	float	min_distance;
};

/*
 * DARKMISS [MIS,SAV] [1,2]
 * Mission number and resource path.
 * Engine Structure: sMissionData
 */
#pragma pack(push,1)
struct DarkDBChunkDARKMISS
{
	sint32	number;
	char	path[9];
};
#pragma pack(pop)

/*
 * DIFFPARAM [GAM] [S]
 * Difficulty base stats.
 * Does "Rep" mean replicator, or repair?
 * Engine Structure: sDiffParams
 */
struct DarkDBChunkDIFFPARAM
{
	float	upgrade_mult[6];
	sint32	base_health[6];
	sint32	endurance_add[6];
	sint32	base_psi[6];
	sint32	psi_add[6];
	sint32	loot_hose[6];
	float	rep_cost[6];
};
#define SHOCKDIFF_PLAYTEST		0
#define SHOCKDIFF_EASY			1
#define SHOCKDIFF_NORMAL		2
#define SHOCKDIFF_HARD			3
#define SHOCKDIFF_IMPOSSIBLE	4
#define SHOCKDIFF_MULTIPLAYER	5

/*
 * DISTOBJVAR [MIS,SAV] [2]
 * T2 New Sky Distant Art.
 * Engine Structure: sMissionDistantObj
 */
struct DarkDBChunkDISTOBJVAR
{
	Boolean	enable;
	Boolean	fog;
	char	bitmap1[256];
	char	bitmap2[256];
	Color	color;
	float	distance;
	float	top;
	float	bottom;
	sint32	panels;
	sint32	panels_per_texture;
	float	alpha;
};

/*
 * GameSysEAX [GAM] [*]
 * MissionEAX [MIS,SAV] [*]
 * Default EAX value for mission/gamesys.
 * Used for chunks MissionEAX and GameSysEAX.
 * Engine Structure: sAcousticsProperty
 */
struct DarkDBChunkEAX
{
	uint32	eax;
	sint32	dampeningfactor;
	sint32	heightoverride;
};
#define EAX_GENERIC		0
#define EAX_SMALLDEAD	1
#define EAX_PADDEDCELL	EAX_SMALLDEAD
#define EAX_SMALLNORMAL	2
#define EAX_ROOM		EAX_SMALLNORMAL
#define EAX_BATHROOM	3
#define EAX_LIVINGROOM	4
#define EAX_LARGENORMAL	5
#define EAX_STONEROOM	EAX_LARGENORMAL
#define EAX_AUDITORIUM	6
#define EAX_CONCERTHALL	7
#define EAX_LARGELIVE	8
#define EAX_CAVE		EAX_LARGELIVE
#define EAX_ARENA		9
#define EAX_CAVERNS		EAX_ARENA
#define EAX_HANGAR		10
#define EAX_DEADHALLWAY	11
#define EAX_CARPETEDHALLWAY	EAX_DEADHALLWAY
#define EAX_NORMALHALLWAY	12
#define EAX_HALLWAY		EAX_NORMALHALLWAY
#define EAX_LIVEHALLWAY	13
#define EAX_STONECORRIDOR	EAX_LIVEHALLWAY
#define EAX_TUNNELS		14
#define EAX_ALLEY		EAX_TUNNELS
#define EAX_OUTSIDE		15
#define EAX_FOREST		EAX_OUTSIDE
#define EAX_CITY		16
#define EAX_MOUNTAINS	17
#define EAX_QUARRY		18
#define EAX_LARGEDEAD	19
#define EAX_PLAIN		EAX_LARGEDEAD
#define EAX_SMALLLIVE	20
#define EAX_PARKINGLOT	EAX_SMALLLIVE
#define EAX_SEWERS		21
#define EAX_SEWERPIPE	EAX_SEWERS
#define EAX_UNDERWATER	22
#define EAX_DRUGGED		23
#define EAX_DIZZY		24
#define EAX_PSYCHOTIC	25

/*
 * Elev [GAM] [S]
 * Elevator locations.
 * Can you have more than five floors?
 * Engine Structure: sElevParams
 */
struct DarkDBChunkElev
{
	char	deck[5][64];
};

/*
 * EXPLORED [MIS,SAV] [S]
 * ????? Rooms Explored.
 * Engine Structure: sExplored
 */
struct DarkDBChunkEXPLORED
{
	char	area[64];
};

/*
 * FAMILY [MIS] [*]
 * Fixed-length array of family names.
 * (For palette management, I think.)
 */
struct DarkDBChunkFAMILY
{
	uint32	size;	// size of each entry: 0x18
	uint32	count;	// number of entries: 0x12
	char	fam[18][24];
	// The first two entries are reserved for water families
};

/*
 * FILE_TYPE [*] [*]
 * Magic-number of file.
 * Seems to be a bitmap representing what chunks might
 * appear in the file, but I can't find any specific associations.
 * Some archaic COWs I have used only the high-word.
 */
struct DarkDBChunkFILE_TYPE
{
	uint32	type;
};
#define FILE_TYPE_VBR	0x00000500
#define FILE_TYPE_SAV	0x00011900
#define FILE_TYPE_MIS	0x00031900
#define FILE_TYPE_GAM	0x00040200
#define FILE_TYPE_COW	0x00071F00
#define FILE_TYPE_MASK_ARCHAIC	0xFFFF0000
// bit 8 - not on GAM
// bit 9 - only on GAM
// bit 10 - only on VBR
// bit 11 - only on MIS/SAV
// bit 12 - only on MIS/SAV
// bit 16 - MIS/SAV
// bit 17 - MIS
// bit 18 - GAM

/*
 * FLOW_TEX [MIS] [*]
 * Association of flow textures to flow colors.
 */
struct DarkDBChunkFLOW_TEX
{
	struct 
	{
		sint16	in_texture;		// index in texture palette
		sint16	out_texture;	// index in texture palette
		char	name[28];
	} flow[256];
};

/*
 * FOG [MIS] [2]
 * T2 fogging.
 * Actual cell fogging data is stored in WRRGB, naturally.
 */
struct DarkDBChunkFOG
{
	sint32	red;
	sint32	green;
	sint32	blue;
	float	distance;
};

/*
 * GAM_FILE [VBR,MIS,SAV] [*]
 * Name of gamesys associated with this mission.
 * Obviously not needed for COW or GAM.
 */
struct DarkDBChunkGAM_FILE
{
	char	name[256];
};

/*
 * GAMEPARAM [GAM] [S]
 * Game Parameters.
 * Engine Structure: sGameParams
 */
struct DarkDBChunkGAMEPARAM
{
	float	power;
	float	bash[8];
	float	speed[8];
	float	overlay_dist;
	float	frob_dist;
};

/*
 * GHOSTREM [VBR,MIS,SAV] [2,S]
 * ?????
 * This one is really odd.
 * The chunk is 4-bytes long, but the inventory lists it as being empty.
 */
struct DarkDBChunkGHOSTREM
{
	sint32	unknown;
};

/*
 * GUNANIM [GAM] [S]
 * Gun animation.
 * Engine Structure: sGunAnimParams
 */
#pragma pack(push,2)
struct DarkDBChunkGUNANIM
{
	sint16	swing_size;
	sint32	swing_time;
	sint16	swing_speed;
	float	bob_size;
	float	bob_speed;
	sint16	raise_speed;
	sint16	raised;
	sint16	lowered;
	float	wobble;
};
#pragma pack(pop)

/*
 * HRM [GAM] [S]
 * Tech Params
 * Hack/Repair/Modify (His Royal Majesty?)
 * Engine Structure: sHRMParams
 */
struct DarkDBChunkHRM
{
	typedef struct
	{
		sint32	skill;
		sint32	stat;
	} DarkDBBonus;
	DarkDBBonus	critfail;
	DarkDBBonus	success;
	float	break_chance[8];
};

/*
 * IMPLPARAM [GAM] [S]
 * Implant worm blend multiplier.
 * I still think the whole worm thing is disgusting.
 * Engine Structure: sImplantParams
 */
struct DarkDBChunkIMPLPARAM
{
	sint32	vis_multiply;
};

/*
 * LOGTIMES [SAV] [C]
 * ????? PDA History.
 * Engine Structure: sLogTimes
 */
struct DarkDBChunkLOGTIMES
{
	uint32	unknown;
	uint32	extra[45][64];
};

/*
 * MAP_FILE [GAM] [*]
 * ?????
 * Used by gamesys.
 */
struct DarkDBChunkMAP_FILE
{
	char	name[256];
};

/*
 * MAPISRC [MIS,SAV] [2]
 * Automap Info.
 * Engine Structure: sMapSourceData
 */
struct DarkDBChunkMAPISRC
{
	uint32	sourcemission;
	float	compassoffset;
};

/*
 * MAPPARAM [MIS,SAV] [S]
 * HUD Map Parameters.
 * Engine Structure: sMapParams
 */
struct DarkDBChunkMAPPARAM
{
	Boolean	rotate_hack;
};

/*
 * MBHEAD [VBR] [*]
 * Multibrush info.
 */
struct DarkDBChunkMBHEAD
{
	uint32	unknown;
};

/*
 * MELEESTR [GAM] [S]
 * Melee strength.
 * Engine Structure: sMeleeStrengthParams
 */
struct DarkDBChunkMELEESTR
{
	sint32	strength[8];
};

/*
 * MISCDATA [SAV] [C]
 * ?????
 * Engine Structure: sMiscData
 */
struct DarkDBChunkMISCDATA
{
	uint32	unknown;
};

/*
 * [SAV] [1,2]
 * Mission Loop info.
 * Only stored in SAV database.
 * SS2 might recognize it, even if it's not used in the regular game.
 * MISSTART saves a version of this with the NOLOADOUT flag set, and the 
 * modes both set to NEWGAME.
 *
 * Engine Structure: sMissLoopState
 */
struct DarkDBChunkMISSLOOP
{
	uint32	current_mission;	// current mission number
	uint32	flags;				// 0x00
	uint32	current_mode;		// 0x0A
	uint32	next_mode;
	uint32	next_mission;		// next mission number
};
/* flags */
#define MISSLOOP_SKIP		0x1
#define MISSLOOP_NOBRIEFING	0x2
#define MISSLOOP_NOLOADOUT	0x4
#define MISSLOOP_CUTSCENE	0x8
#define MISSLOOP_END		0x10
/* modes */
 /* Nowhere */
#define MISSLOOP_NULL		0
 /* Before the main menu appears. */
#define MISSLOOP_APPSTART	1
 /* The main menu. */
#define MISSLOOP_MAINMENU	2
 /* The new game menu. */
#define MISSLOOP_NEWGAME	3
 /* The metagame menu, also if select_missions is set. */
#define MISSLOOP_METAGAME	4
 /* Read the mission flags and next mission, skipping if necessary. */
#define MISSLOOP_MISSFLAG	5
 /* The briefing movie and text. */
#define MISSLOOP_BRIEFING	6
 /* Spinning dials. */
#define MISSLOOP_LOADING	7
 /* Set difficulty, create Player, prepare loot. */
#define MISSLOOP_STARTMISSION	8
 /* Inventory loadout screen. */
#define MISSLOOP_LOADOUT	9
 /* Start the simulation. */
#define MISSLOOP_STARTSIM	10
 /* End the simulation. */
#define MISSLOOP_ENDSIM		11
 /* End-of-mission movies. */
#define MISSLOOP_ENDMISSION	12
 /* Statistics/objectives screens. */
#define MISSLOOP_DEBRIEF	13
 /* Continue to next mission, or skip to RESTORE mode. */
#define MISSLOOP_MISSIONCOMPLETE	19
 /* Cutscene movies. */
#define MISSLOOP_CSMOVIE	14
 /* No more missions. */
#define MISSLOOP_ENDGAME	15
 /* Do nothing. */
#define MISSLOOP_DEFAULT	16
 /* Jump back to NEWGAME. */
#define MISSLOOP_RESTART	17
 /* Reload the current mission, then start again. */
#define MISSLOOP_RESTORE	18


/*
 * MISSTART [SAV] [1,2]
 * Quick-save data for restarting the current mission.
 * The chunk is a mini-DarkDB with the vital chunks, 
 * MISSLOOP, QUEST_DB, PlayerInv, etc.
 */

/*
 * MSGHISTORY [SAV] [C]
 * Recently displayed messages.
 * Engine Structure: sMsgHistory
 */
struct DarkDBChunkMSGHISTORY
{
	char	message[18][255];
	sint32	unknown[18];
};

/*
 * ObjVec [*] [*]
 * Allocation bitmap for object IDs.
 * bit[(id-min_id)/8] & 1<<(id%8)
 * But this is a hella lot faster:
 * bit[(id-min_id)>>3] & 1<<((id-min_id)&0x7)
 * Not using divs is a Good Thing(tm).
 */
struct DarkDBChunkObjVec
{
	sint32	min_id;		// Each ID is rounded up to a multiple of 8
	sint32	max_id;		// So ID 0 will always be bit#0 of some byte
	uint8	bitmap[];
};

/*
 * OLPARAM [GAM] [S]
 * Overload info.
 * Engine Structure: sOverloadParams
 */
struct DarkDBChunkOLPARAM
{
	float	burn_factor[8];		// Endurance
	float	burnout_time[5];
	float	psi_threshold[8];
	sint32	damage;		// per-level
};

/*
 * OVERLAY [SAV] [C]
 * ????? Overlay State.
 * Engine Structure: sRestoreInfo
 */
struct DarkDBChunkOVERLAY
{
	char	unknown[204];
};

/*
 * OW Actions
 * OverWorld Actions.
 * One of the SShock2 mini-games.
 * Engine Structure: sSaveActions
 */
struct DarkDBChunkOWActions
{
};

/*
 * OW Monsters
 * OverWorld Monster.
 * One of the SShock2 mini-games.
 * Engine Structure: sSaveMonsters
 */
struct DarkDBChunkOWMonsters
{
	uint8	num_monsters;
	struct 
	{
		sint8	type;
		uint8	hp;
		uint8	flags;
		uint8	zero;
		uint8	xpos,ypos;
		uint8	xorig,yorig;
	} creature[128];
};
#define OWMONSTER_PLAYER	0
#define OWMONSTER_GOBLIN	1
#define OWMONSTER_HEADLESS	2
#define OWMONSTER_BAT	3
#define OWMONSTER_SWINE	4
#define OWMONSTER_TROLL	5
#define OWMONSTER_GHOST	6
#define OWMONSTER_SQUID	7
#define OWMONSTER_DEMON	8
#define OWMONSTER_DRAGON	9
#define OWMONSTER_WISEMAN	10
#define OWMONSTER_GAZER	11
#define OWMONSTER_REAPER	12
#define OWMONSTER_MIMIC	13
#define OWMONSTER_LURKER	14
#define OWMONSTER_SLIME	15
#define OWMONSTER_MINGBAT	16
#define OWMONSTER_SNICKERS	17
#define OWMONSTER_LB	18
#define OWMONSTER_MERCHANT	19

/*
 * OW Player
 * OverWorld Player.
 * One of the SShock2 mini-games.
 * Engine Structure: sSavePlayer
 */
struct DarkDBChunkOWPlayer
{
	uint32	mode;		/* 1 = normal, 2 = dead, 3 = won */
	uint8	xpos,ypos;
	uint8	xorig,yorig;
	uint8	xwhat,ywhat;
	uint8	level;
	sint8	countdown;
	uint8	flags;
	uint8	potions;
	uint8	quest;
	uint8	completed;
	uint8	items;
	uint8	dragons[4];
	uint8	slays;
	uint8	prayers;
	uint8	mousedown;
	sint8	mousetime;
	uint8	deaths;
	sint16	unknown;
	sint16	hp;
	sint16	xp;
	sint16	gp;
	sint16	zero;
	sint32	moves;
	char	towns[14];
};
#define OWPLAYER_WISEMANHELLO	1
#define OWPLAYER_WISEMAN	2
#define OWPLAYER_WISEMANBYE	3
#define OWPLAYER_MERCHANTHELLO	4
#define OWPLAYER_MERCHANT	8
#define OWPLAYER_LEVELUPMASK	0xC0

/*
 * OW MStats
 * OverWorld Monster Stats.
 * One of the SShock2 mini-games.
 * Engine Structure: sSaveStats
 */
struct DarkDBChunkOWMStats
{
	struct 
	{
		uint8	numdice_hit;
		uint8	diesides_hit;
		sint8	add_hit;
		uint8	zero1;
		uint8	numdice_hp;
		uint8	diesides_hp;
		sint8	add_hp;
		uint8	zero2;
		uint8	numdice_att;
		uint8	diesides_att;
		sint8	add_att;
		uint8	zero3;
		sint8	defence;
		uint8	expbonus;
		uint8	minlevel;
		sint8	vision;
		uint8	dropchance;
		uint8	item;
		uint8	unknown;
		uint8	terrain;
	} type[20];
};
#define OWMSTATS_TERRAIN_GROUND	1
#define OWMSTATS_TERRAIN_MOUNTAIN	2
#define OWMSTATS_TERRAIN_WATER	4
#define OWMSTATS_TERRAIN_NOFOREST	8
#define OWMSTATS_TERRAIN_ANY	9

/*
 * OW Map
 * OverWorld World Tiles.
 * One of the SShock2 mini-games.
 * Engine Structure: sWorldTiles
 */
struct DarkDBChunkOWMap
{
	struct 
	{
		sint8	type;
		sint8	item;
		sint8	creature;
		sint8	special;
		sint8	desc;
	} tiles[64][64];
};
#define OWTILE_PLAINS	0
#define OWTILE_FOREST	1
#define OWTILE_MOUNTAIN	2
#define OWTILE_WATER	3
#define OWTILE_VILLAGE	4
#define OWTILE_CITY	5
#define OWTILE_ROAD	6
#define OWTILE_SHRINE	7
#define OWTILE_CAMP	8
#define OWTILE_TEMPLE	9
#define OWITEM_1GP	0
#define OWITEM_6GP	1
#define OWITEM_17GP	2
#define OWITEM_ARMOR	3
#define OWITEM_HOE	4
#define OWITEM_SHIELD	5
#define OWITEM_SWORD	6
#define OWITEM_POTION	7
#define OWSPECIAL_DAMAGE	0
#define OWSPECIAL_NODAMAGE	1
#define OWSPECIAL_FIREBALL	2
#define OWSPECIAL_DART	3
/* Add to monster type for camp type. */
#define OWMONSTER_CAMP	100

/*
 * PLAYER [VBR,MIS,SAV] [*]
 * The Player chunk has two forms, the later is used when the avatar object
 * has been created and contains the ID to it. The only difference is the
 * size of the chunk.
 * XXX: SShock2 SAV has a lot more info here, 
 */
struct DarkDBChunkPLAYER
{
	uint32	crouching;
	uint32	unknown2;
};

struct DarkDBChunkPLAYER_Sim
{
	uint32	crouching;
	uint32	unknown2;
	sint32	avatar_id;
};

struct DarkDBChunkSS2PLAYER_Sim
{
	uint32	crouching;
	uint32	unknown2;
	char	name[32];
	float	unknown3;
	float	unknown4;
};

struct DarkDBChunkSS2PLAYER_SimAv
{
	uint32	crouching;
	uint32	unknown2;
	char	name[32];
	float	unknown3;
	float	unknown4;
	sint32	avatar_id;
};

/*
 * PLAYERCAM [VBR,MIS,SAV] [2]
 * Definition of the camera location, et.al.
 */
#pragma pack(push,2)
struct DarkDBChunkPLAYERCAM
{
	uint32	attachment;	// 5 before SIM, 0 after, 3 or 4 in scouting orb mode, 2 when attached to an object
	float	fov;	// 1.0 is normal. >1 contracts fov (zoom in), <1 expands (fisheye)
	Coord	position;
	SCoord	rotation;
	uint16	unknown3[3];	// 0x00
	sint32	attached_id;
};
#pragma pack(pop)

/*
 * PlayerInv [SAV] [1,2]
 * ?????
 */

/*
 * PSICOST [GAM] [S]
 * Psi Costs.
 * The best things in life aren't free.
 * Engine Structure: sPsiCost
 */
struct DarkDBChunkPSICOST
{
	uint32	cost[5][8];
};
#define PSI_LEVEL				0
#define PSI_LEVEL1_FEATHERFALL	1
#define PSI_LEVEL1_STILLHAND	2
#define PSI_LEVEL1_PULL			3
#define PSI_LEVEL1_QUICKNESS	4
#define PSI_LEVEL1_CYBER		5
#define PSI_LEVEL1_CRYOKINESIS	6
#define PSI_LEVEL1_CODEBREAKER	7
#define PSI_LEVEL2_STABILITY	1
#define PSI_LEVEL2_BERSERK		2
#define PSI_LEVEL2_RADSHIELD	3
#define PSI_LEVEL2_HEALING		4
#define PSI_LEVEL2_MIGHT		5
#define PSI_LEVEL2_PSI			6
#define PSI_LEVEL2_IMMOLATE		7
#define PSI_LEVEL3_FABRICATE	1
#define PSI_LEVEL3_ELECTRO		2
#define PSI_LEVEL3_ANTIPSI		3
#define PSI_LEVEL3_TOXINSHIELD	4
#define PSI_LEVEL3_RADAR		5
#define PSI_LEVEL3_PYROKINESIS	6
#define PSI_LEVEL3_TERROR		7
#define PSI_LEVEL4_INVISIBILITY	1
#define PSI_LEVEL4_SEEKER		2
#define PSI_LEVEL4_DAMPEN		3
#define PSI_LEVEL4_VITALITY		4
#define PSI_LEVEL4_ALCHEMY		5
#define PSI_LEVEL4_CYBERHACK	6
#define PSI_LEVEL4_PSISWORD		7
#define PSI_LEVEL5_MAJORHEAL	1
#define PSI_LEVEL5_SOMADRAIN	2
#define PSI_LEVEL5_TELEPORT		3
#define PSI_LEVEL5_ENRAGE		4
#define PSI_LEVEL5_FORCEWALL	5
#define PSI_LEVEL5_PSIMINES		6
#define PSI_LEVEL5_PSISHIELD	7

/*
 * PSITRAIN [SAV] [C]
 * ?????
 * Engine Structure: sPsiTrainData
 */
struct DarkDBChunkPSITRAIN
{
	uint32	unknown;
};

/*
 * QUEST_DB [MIS,SAV] [*]
 * QUEST_CMP [SAV] [C]
 * Quest Variables
 * Used for chunks QUEST_CMP and QUEST_DB.
 */
/* More variable-length crap. I'm all for flexibility,
   but fixed records are a damn sight easier to work with.
struct DarkDBChunkQUEST
{
	uint32	name_length;
	char	name[];
	sint32	value;
};
*/

/*
 * Relations [*] [*]
 * Variable-length array of link names, in ID order
 */
struct DarkDBChunkRelations
{
	char	name[1][32];
};

/*
 * RENDPARAMS [MIS,SAV] [1,S]
 * Thief1 ambient lighting and default palette.
 * Engine Structure: sMissionRenderParams
 */
struct DarkDBChunkRENDPARAMS
{
	char	paletteres[16];
	float	ambient;
};

/*
 * RENDPARAMS [MIS,SAV] [2]
 * Thief2 ambient light, sunlight, and default palette.
 */
struct DarkDBChunk2RENDPARAMS
{
	char	paletteres[16];
	Color	ambientRGB;
	Boolean	usesunlight;
	Boolean	quadsunlight;
	Coord	sunlightdirection;
	float	sunlighthue;
	float	sunlightsaturation;
	float	sunlightbrightness;
	float	zero[6];
};

/*
 * ROOM_EAX [MIS,SAV] [*]
 * Array of EAX settings for each room.
 * In room number order. And since the room numbers
 * are regenerated with each build of the room db, 
 * this isn't guaranteed to be consistent.
 */
struct DarkDBChunkROOM_EAX
{
	uint32	count;
	uint32	eax[];
};

/*
 * SAVEDESC [SAV] [1,2,C]
 * String describing a saved game.
 * Talk about overkill.
 */
struct DarkDBChunkSAVEDESC
{
	char	description[1024];
	uint32	extra;
};
struct DarkDBChunkSAVEDESC_SS2
{
	char	description[1024];
	char	mission[32];
	uint32	extra;
};

/*
 * SchSamp [GAM] [*]
 * Array of array of samples for schemas.
 */
struct DarkDBChunkSchSamp
{
	sint32	id;		// schema id
	uint32	count;	// number of samples
/*
	struct 
	{
		uint32	name_length;	// length of sample name
		char	name[];		// sample name
		uint8	priority;		// play priority
	};
*/
};

/*
 * ENV_SOUND [GAM] [*]
 * Schema tag database.
 * The schema ids associated with a set of tags 
 * is stored in a depth-first three.
 * Also includes a list of required tags.
 */
/*
 * Speech_DB [GAM] [*]
 * Concept, voice, and schema tags
 * Contains concept and tag/argument names,
 * and schema-to-concept associations.
 * schema-to-tag assocations are elsewhere, probably ENV_SOUND.
 */

/*
 * Each schema association may contain zero or more schemas, 
 * and zero or more subordinate tags. The tags form a depth-first 
 * tree structure.
 * Tags can have an argument of either two signed integers, or at most
 * eight indexes to the tag argument list. In the later case, the array
 * or arguments is terminated by 0xFF (putting a limit of 255 on the number
 * of defined arguments). Which argument style to use is determined by the 
 * second uint32 list after the three tag lists.
 * After the tag arguments is a schema association, which may include more 
 * subordinate tags that will inherit this tag's definition.
 *
 * The tag and argument definitions for ENV_SOUND are in Speech_DB.
 */
struct DarkDBSchemaTagEntry
{
	uint32	tag;	// 0-based index
	union {
		sint32	i[2];
		uint8	t[8];	// 0-based index, 255 is reserved
	} args;
};

struct DarkDBSchemaEntry
{
	uint32	count;
	struct 
	{
		sint32	id;
		float	priority;
	} schemas[];
/*
	uint32	tag_count;
	struct 
	{
		DarkDBSchemaTagEntry	tag;
		DarkDBSchemaEntry	schema;
	} tags;
*/
};

/*
 * The root entry is (should?) have zero schemas defined,
 * only subordinate tags.
 */
 struct DarkDBChunkENV_SOUND
 {
	uint32	required_count;
	uint8	required_tags[];	// 1-based index
	/*
	DarkDBSchemaEntry	root;
	*/
 };

/*
 * Arrays of speech broadcasts, device types, tags, etc.
 * Speech_DB begins with three of these: concepts, tags, tag arguments.
 * Then there's two count+array of uint32 associated with
 * the first two Tag lists: concept priority, tag type (1=int,2=arg).
 * Then there is an uint32 of the number of voices, followed by 
 * an array of voice concept associations for each voice, in order of index.
 * 
 * For each voice, there is an array of schema entries for all concepts,
 * in the order that the concepts are listed previously.
 */
#pragma pack(push,1)
struct DarkDBSchemaTagList
{
	sint32	something;	// count - 1, highest index I suppose
	uint32	zero;
	uint32	count;
	char	name[][17];
};
#pragma pack(pop)

/*
 * SCRIPTSTATE [MIS,SAV] [*]
 * Saved state of scripts.
 */
/* Another funky variable-length list
 * This really sucks.
 * script_params probably stores data, like strings and vectors, 
 * that are larger than 4 bytes. If I were to guess, I'd say they 
 * have an entry in script_params that give the type and offset 
 * to the data.
struct DarkDBChunkSCRIPTSTATE
{
	uint32	unknown1;	// 0x00
	uint32	unknown2;	// 0x00
	uint32	count;	// number of param-based entries
	struct 
	{
		sint32	id;			// object id
		uint32	name_length;
		char	name[];	// script name WITHOUT terminating null
		uint32	param_length;
		char	param[]; // param name WITHOUT terminating null
		uint32	type;
		sint32	value;
	} script_params[];
	struct 
	{
		sint32	id;
		char	name[];	// null-terminated script name
		uint8	data[];	// arbitrary data
	} script_data[];
};
*/

/*
 * ScrModules [MIS,SAV] [*]
 * Variable-length array of script module names.
 */
struct DarkDBChunkScrModules
{
	char	name[1][128];
};

/*
 * SIM_MAN [MIS,SAV] [*]
 * ?????
 */
struct DarkDBChunkSIM_MAN
{
	sint32	unknown;
};

/*
 * SIM_TIME [MIS,SAV] [*]
 * How long this mission has been running.
 */
struct DarkDBChunkSIM_TIME
{
	uint32	time;		// milliseconds
	uint32	unknown;
};

/*
 * SKILLPARAM [GAM] [S]
 * Skill modifiers.
 * Engine Structure: sSkillParams
 */
#pragma pack(push,2)
struct DarkDBChunkSKILLPARAM
{
	sint16	inaccuracy;
	float	break_mod;
	float	research_factor;
	float	weapon_damage;
	float	organ_damage;
};
#pragma pack(pop)

/*
 * SKYMODE [MIS,SAV] [*]
 * How to render the sky.
 * Engine Structure: sSkyMode
 */
struct DarkDBChunkSKYMODE
{
	uint32	mode;
};
#define SKYMODE_TEXTURE		0
#define SKYMODE_STARS		1

/*
 * SKYOBJVAR [MIS,SAV] [2]
 * T2 New Sky
 * Engine Structure: sMissionSkyObj
 */
struct DarkDBChunkSKYOBJVAR
{
	Boolean	enable;
	Boolean	fog;
	float	atmosphere_radius;
	float	earth_radius;
	sint32	latitude_count;
	sint32	longitude_count;
	float	dip_angle;
	Color	pole_color;
	Color	fortyfive_color;
	Color	seventy_color;
	Color	horizon_color;
	Color	dip_color;
	Color	glow_color;
	float	glow_latitude;
	float	glow_longitude;
	float	glow_angle;
	float	glow_scale;
	uint32	glow_method;
	float	clip_latitude;
};

/*
 * SndScript [MIS,SAV] [*]
 * ?????
 */
struct DarkDBChunkSndScript
{
	sint32	unknown;
};

/*
 * SONGTHEME [SAV] [C]
 * Name of background music current theme.
 * Are you sure this isn't in Thief2?
 * Engine Structure: sThemeSaveLoad
 */
struct DarkDBChunkSONGTHEME
{
	char	name[32];
};

/*
 * SONGPARAMS [MIS,SAV] [2,S]
 * Name of background music for the mission.
 * Engine Structure: sMissionSongParams
 */
struct DarkDBChunkSONGPARAMS
{
	char	name[32];
};

/*
 * STAROBJVAR [MIS,SAV] [2]
 * T2 New SKy Stars.
 * Engine Structure: sMissionStarObj
 */
struct DarkDBChunkSTAROBJVAR
{
	Boolean	enable;
	Boolean	fog;
	float	density;
	float	offset;
	float	intensity;
};

/*
 * STATCOST [GAM] [S]
 * Cost to move up a level.
 * Starting level is 1, so there's only 5 more to go.
 * Engine Structure: sStatCost
 */
struct DarkDBChunkSTATCOST
{
	sint32	strength[5];
	sint32	endurance[5];
	sint32	psi[5];
	sint32	agility[5];
	sint32	cyber[5];
};

/*
 * STATPARAM [GAM] [S]
 * Stat Parameters.
 * Engine Structure: sStatParams
 */
struct DarkDBChunkSTATPARAM
{
	sint32	health_start;
	sint32	health_add;		// per endurance
	sint32	psi_start;
	sint32	psi_add;
	float	vis_min;		// for cameras
	float	vis_max;
	float	hazard[8];
};

/*
 * TRAITPARAM [GAM] [S]
 * Trait Bonuses.
 * Engine Structure: sTraitParams
 */
struct DarkDBChunkTRAITPARAM
{
	sint32	tank_bonus;
	float	sharpshooter;
	float	lethalweapon;
};

/*
 * TXLIST [MIS] [*]
 * Index of textures and the families they're from.
 */
struct DarkDBChunkTXLIST
{
	uint32	length;		// length of TXLIST
	uint32	txt_count;		// number of individual textures
	uint32	fam_count;	// number of families
	// array of family names; first entry is "fam"
	// array of DarkDBTXLIST_texture; first entry is "null"
};

/*
 * Single family name in TXLIST.
 */
struct DarkDBTXLIST_fam
{
	char	name[16];
};

/*
 * Single texture name in TXLIST.
 */
struct DarkDBTXLIST_texture
{
	uint8	one;		// 0x01 (except on "null" texture)
	uint8	fam;		// number of family (one-based count, 0 for no fam)
	uint16	zero;		// 0x00
	char	name[16];	// texture name
};

/*
 * TXTPAT_DB	// [MIS,SAV] [*]
 * ?????
 */
struct DarkDBChunkTXTPAT_DB
{
	uint32	unknown;
};

/*
 * VISITED [SAV] [2]
 * List of visited rooms, for automapping.
 * Engine Structure: sVisited
 */
struct DarkDBChunkVISITED
{
	sint32	room[2048];
};

/*
 * Single entry in WATERBANKS.
 * Engine Structure: sRGBA
 */
struct DarkDBWaterColor
{
	uint8	red;
	uint8	green;
	uint8	blue;
	uint8	zero;
	float	alpha;
};

/*
 * WATERBANKS [MIS,SAV] [*]
 * Water colors mission parameter.
 * Engine Structure: sWaterBanks
 */
struct DarkDBChunkWATERBANKS
{
	struct DarkDBWaterColor color[4];
};

/*
 * WEATHER [VBR,MIS,SAV] [2]
 * T2 Precipitation DB
 * List of weather flags for each renderer cell.
 * Only tracks precipitation. Fog is only in WRRGB.
 * Rain might also be in WRRGB.
 */
struct DarkDBChunkWEATHER
{
	uint8	cell[];
};
#define WEATHER_FLAG_PRECIPITATION	2

/*
 * WEATHERVAR [MIS,SAV] [2]
 * T2 Weather.
 * Engine Structure: sMissionWeather
 */
struct DarkDBChunkWEATHERVAR
{
	uint32	type;
	float	frequency;	// new drops per sec
	float	velocity;	// feet per sec
	float	visible_distance;	// feet
	float	radius;		// feet
	float	alpha;		// 0-1
	float	brightness;		// 0-1
	float	snow_jitter;	// feet
	float	rain_length;	// feet
	float	splash_frequency;	// 0-1
	float	splash_radius;	// feet
	float	splash_height;	// feet
	float	splash_time;	// sec
	char	texture[32];
	Coord	wind;	// feet per sec
};
#define WEATHER_TYPE_SNOW	0
#define WEATHER_TYPE_RAIN	1

/*
 * WSKILLCOST [GAM] [S]
 * Weapon skill cost.
 * Engine Structure: sWeaponSkillCost
 */
struct DarkDBChunkWSKILLCOST
{
	sint32	conventional[6];
	sint32	energy[6];
	sint32	heavy[6];
	sint32	alien[6];
};

/*
 * WTECHCOST [GAM] [S]
 * Tech skill cost.
 * Engine Structure: sTechKillCost
 */
struct DarkDBChunkWTECHCOST
{
	sint32	hack[6];
	sint32	repair[6];
	sint32	modify[6];
	sint32	maintenance[6];
	sint32	research[6];
};


/*
 * BRLIST [VBR,MIS] [*]
 * Single BRLIST entry.
 */
#pragma pack(push,2)
struct DarkDBBrushFace
{
	sint16	texture;
	uint16	rotation;	// 1 = ALIGN_BY_BRUSH
	uint16	scale;
	uint16	offset[2];
};
struct DarkDBBrush
{
	uint16	id;
	uint16	time;		// time is wierd
	sint32	primal;		// terrain:shape, room:oid, object:oid, light:light number
	sint16	base;		// terrain:texture, room:room number, flow:gid, area:active
	sint8	type;
	uint8	zero1;		// unused
	Coord	position;	// center
	Coord	size;		// length/2,width/2,height/2 or brightness,hue,saturation
	SCoord	rotation;	// tx,ty,tz
	sint16	cur_face;	// selected face
	float	snap_size;		// size of grid to snap to (14=1.0)
	uint8	unknown3[18];
	uint8	snap_grid;		// vertices are snapped to grid
	uint8	num_faces;		// or type of light
	uint8	edge;
	uint8	vertex;
	uint8	flags;		// damn trickery
	uint8	group;
	uint32	unknown5;
	struct DarkDBBrushFace	faces[];
	// Optional array of struct DarkDBBrushFace
};
#pragma pack(pop)
// 
#define BRUSH_SHAPE_MASK		0xFF00
#define BRUSH_SHAPE_CUBE		0x0001
#define BRUSH_SHAPE_DODECAHEDRON	0x0006
#define BRUSH_SHAPE_WEDGE		0x0007
#define BRUSH_SHAPE_CYLINDER		0x0200
#define BRUSH_SHAPE_PYRAMID		0x0400
#define BRUSH_SHAPE_APEXPYRAMID		0x0600
#define BRUSH_ALIGN_VERTICES		0x0000
#define BRUSH_ALIGN_SIDES		0x0100
#define BRUSH_TYPE_SOLID		0
#define BRUSH_TYPE_AIR			1
#define BRUSH_TYPE_WATER		2
#define BRUSH_TYPE_AIR2WATER		3
#define BRUSH_TYPE_FLOOD		BRUSH_TYPE_AIR2WATER
#define BRUSH_TYPE_WATER2AIR		4
#define BRUSH_TYPE_EVAPORATE		BRUSH_TYPE_WATER2AIR
#define BRUSH_TYPE_SOLID2WATER		5
#define BRUSH_TYPE_SOLID2AIR		6
#define BRUSH_TYPE_AIR2SOLID		7
#define BRUSH_TYPE_WATER2SOLID		8
#define BRUSH_TYPE_BLOCKABLE		9
#define BRUSH_TYPE_ROOM			-5
#define BRUSH_TYPE_FLOW			-4
#define BRUSH_TYPE_OBJECT		-3
#define BRUSH_TYPE_AREA			-2
#define BRUSH_TYPE_LIGHT		-1
#define BRUSH_LIGHT_OMNI		0
#define BRUSH_LIGHT_SPOT		1
#define BRUSH_AREA_ACTIVE		1
#define BRUSH_AREA_MEONLY		2
#define BRUSH_FLAG_ALIGN_BY_SIDES	0
#define BRUSH_FLAG_ALIGN_BY_VERTICES	4
#define BRUSH_FLAG_ANCHOR_VERTEX	4
#define BRUSH_FLAG_ANCHOR_EDGE		6
#define BRUSH_FLAG_LIGHT_SPOT		0
#define BRUSH_FLAG_LIGHT_OMNI		1


/*
 * A list of integers, used a few times in room databases
 * count is the number entries in the ID list, and may be zero.
 */
struct DarkDBIDList
{
	uint32	count;
	sint32	id[];
};

/*
 * ROOM_DB [MIS,SAV] [*]
 * Room database
 */
struct DarkDBChunkROOM_DB
{
	uint32	version;	// 0x01
	uint32	num_rooms;
	// array of DarkDBRoom + DarkDBRoomPortal entries
};
#pragma pack(push,2)
struct DarkDBRoom
{
	sint32	id;		// object ID
	sint16	room;		// room number
	Coord	center;		// should not be in solid space or overlapping another room
	Plane	plane[6];
	uint32	num_portals;
	//DarkDBRoomPortal portals[]
	// A matrix of portal-to-portal distances, calculated as the linear distance
	// between each pair of portal centers.
	//float distances[] // num_portals**2
	//uint32	num_lists // The number of count+arrays that follow. Usually 2.
	//DarkDBIDList lists[]
	// There are usually two lists.
	// The first is the IDs of all objects that are in this room.
	// The second is the object IDs of creatures in this room.
	// (The IDs should be in both lists, unless you know of an AI that isn't an object.)
};
#pragma pack(pop)

struct DarkDBRoomPortal
{
	sint32	id;		// portal ID
	uint32	index;		// The index of this portal in the room's portal list
	Plane	plane;
	uint32	num_edges;
	Plane	edge[];
};
struct DarkDBRoomPortal_tail
{
	sint32	dest_room;	// room number this portal goes to
	sint32	src_room;	// this room number
	Coord	center;		// center point of the portal. (should not be in solid space)
	sint32	dest_portal;	// portal ID on the other side of this portal
};


/*
 * AI_ROOM_DB [MIS,SAV] [*]
 * AI Cell Rooms
 */
struct DarkDBChunkAI_ROOM_DB
{
	uint32	zero;
	sint32	max_cell;	// number of cells (from AIPATH) + 2
	uint32	num_rooms;	// should be the same as in ROOM_DB
	//DarkDBIDList	lists[] // the cell IDs in each room
};


/*
 * Chunks which need more intelligent parsing.
 * (Or a more intelligent person to disassemble.)
 *
 * AIPATH		// [MIS] [*] AI Pathfinding
 * CELL_MOTION	// [MIS] [*] ?????
 * CRET_SYSTEM	// [VBR,MIS,SAV] [*] ????? (creatures)
 * PHYS_SYSTEM	// [VBR,MIS,SAV] [*] ?????
 * WR/WRRGB		// [MIS] [1]/[2,S] WorldRep - Understand this and you can do away with Dromed
 */

#ifdef __cplusplus
} // namespace Dark
#endif

#endif
