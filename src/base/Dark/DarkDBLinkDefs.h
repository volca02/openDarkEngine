/******************************************************************************
 *    DarkDBLinkDefs.h
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

#ifndef DARK_DARKDBLINKDEFS_H
#define DARK_DARKDBLINKDEFS_H

#ifdef __cplusplus
namespace Dark {
#endif

struct DarkDBLinkAIAttack {
    uint32 priority;
};

struct DarkDBLinkAIAwareness {
    sint32 unknown1;
    uint32 flags;
    uint32 level;
    uint32 peaklevel;
    sint32 levelenter;
    sint32 lastcontact;
    Coord lastposition;
    sint32 unknown2;
    sint32 visioncone;
    sint32 lastupdate;
    sint32 lastseen;
    sint32 unknown3;
    sint32 lastfirsthand;
    sint32 freshness;
    sint32 unknown4;
};

struct DarkDBLink1AIAwareness {
    sint32 unknown1;
    uint32 flags;
    uint32 level;
    uint32 peaklevel;
    sint32 levelenter;
    sint32 lastcontact;
    Coord lastposition;
    sint32 unknown2;
    sint32 visioncone;
    sint32 lastupdate;
    sint32 lastseen;
    sint32 unknown3;
    sint32 lastfirsthand;
    sint32 freshness;
};

struct DarkDBLinkAICamera {
    char message[32];
    char data[3][32];
};

struct DarkDBLinkAIConversationActor {
    sint32 actor;
};

struct DarkDBLinkAIDefendObj {
    sint32 unknown1[14];
    uint32 returnspeed;
    sint32 unknown2[20];
    struct {
        sint32 radius;
        sint32 height;
        uint32 minalertness;
        uint32 maxalertness;
        sint32 unknown3[4];
    } range[3];
};

struct DarkDBLinkAIDoor {
    sint32 unknown;
};

struct DarkDBLinkAIFleeDest {
    sint32 unknown;
    Boolean reached;
    sint32 expiration;
};

struct DarkDBLinkAIFollowObj {
    sint32 unknown1[2];
    struct {
        float angle;
        sint32 distance;
    } _vector[3];
    sint32 unknown2[2];
};

struct DarkDBLinkAINoFlee {
    sint32 expiration;
};

struct DarkDBLinkAIProjectile {
    uint32 constraint;
    sint32 data;
    uint32 method;
    uint32 priority;
    float delay;
    Boolean leading;
    sint32 ammo;
    sint32 burst;
    uint32 accuracy;
    uint32 joint;
    sint32 unknown[3];
};
#define PROJECTILECONSTRAINT_NONE 0
#define PROJECTILECONSTRAINT_NEARBYAI 1
#define PROJECTILECONSTRAINT_MISSES 2
#define PROJECTILEMETHOD_STRAIGHTLINE 0
#define PROJECTILEMETHOD_ARCING 1
#define PROJECTILEMETHOD_REFLECTING 2
#define PROJECTILEMETHOD_OVERHEAD 3

struct DarkDBLinkAISuspiciousLink {
    sint32 timeseen;
};

struct DarkDBLinkAIWatchObj {
    sint32 unknown[15];
    uint32 trigger;
    uint32 awareness;
    uint32 visibility;
    sint32 unknown2;
    uint32 killcondition;
    Boolean killlikelinks;
    uint32 onceonly;
    sint32 reusetime;
    sint32 resettime;
    uint32 minalertness;
    uint32 maxalertness;
    uint32 priority;
    sint32 radius;
    sint32 height;
    struct DarkPScriptCommand step[7];
    uint8 zero[260];
    /*uint8	extra[2080];	Even more padding in Dark2, leave it up to the app to
     * fill out */
};

struct DarkDBLinkARSrc {
    sint32 sourcelink;
    sint32 starttime;
    sint32 count;
};

struct DarkDBLinkARSrcDesc {
    sint32 sourcetype;
    float intensity;
    uint32 validfields;
    union {
        struct {
            uint32 types;
            float velocitycoeff;
            float frobtimecoeff;
            sint32 zero[5];
        } contact;
        struct {
            float distance;
            uint32 flags;
            uint32 dispersion;
            sint32 zero[5];
        } radius;
    } shape;
    struct {
        uint32 flags;
        sint32 period;
        sint32 maxfirings;
        float slope;
        sint32 zero[4];
    } lifecycle;
    char type[32];
};
#define STIMSOURCE_NONE 0
#define STIMSOURCE_CONTACT 1
#define STIMSOURCE_RADIUS 2
#define STIMSOURCE_FLOW 3
#define STIMSOURCE_SCRIPT 4
/* the bits */
#define STIMFIELDS_SHAPE 1
#define STIMFIELDS_LIFECYCLE 2
/* standard values */
#define STIMFIELDS_CONTACT STIMFIELDS_SHAPE
#define STIMFIELDS_RADIUS STIMFIELDS_SHAPE | STIMFIELDS_LIFECYCLE
#define STIMFIELDS_FLOW STIMFIELDS_LIFECYCLE
#define STIMFIELDS_SCRIPT STIMFIELDS_LIFECYCLE

/* In T2, this is an enum. In SS2 it's an integer. In T1 it's nothing. */
struct DarkDBLinkContains {
    sint32 location;
};

struct DarkDBLinkCorpse {
    Boolean propagatestim;
};

/* XXX: probably a angle/distance vector */
struct DarkDBLinkCreatureAttachment {
    uint32 joint;
    Boolean isweapon;
    float unknown1;
    sint32 unknown2[3];
    float unknown3;
    sint32 unknown4[3];
    float unknown5;
    sint32 unknown6[3];
    sint32 unknown7;
};

struct DarkDBLinkCurWeapon {
    sint32 unknown;
};

struct DarkDBLinkFlinder {
    sint32 count;
    float impulse;
    Boolean scatter;
    Coord offset;
};

struct DarkDBLinkFlowContact {
    sint32 unknown[4];
};

struct DarkDBLinkFrobProxy {
    uint32 flags;
};
#define FROBPROXY_DOINV 0x01
#define FROBPROXY_NOWORLD 0x02
#define FROBPROXY_DOTOOLSRC 0x04
#define FROBPROXY_NOTOOLDST 0x08
#define FROBPROXY_ALLOWDIRECT 0x10

struct DarkDBLinkGunFlash {
    sint32 vhot;
    uint32 flags;
};

struct DarkDBLinkHitSpang {
    sint32 object;
};

struct DarkDBLinkHostObj {
    sint32 unknown;
};

struct DarkDBLinkLandingPoint {
    sint32 unknown;
};

struct DarkDBLinkLock {
    sint32 unknown;
};

struct DarkDBLinkMetaProp {
    sint32 priority;
};

struct DarkDBLinkMissSpang {
    Boolean normal;
};

struct DarkDBLinkNowPicking {
    sint32 unknown;
};

struct DarkDBLinkParticleAttachment {
    uint32 type;
    sint32 vhot;
    uint32 joint;
    sint32 submodel;
};
#define ATTACHTYPE_OBJECT 0
#define ATTACHTYPE_VHOT 1
#define ATTACHTYPE_JOINT 2
#define ATTACHTYPE_SUBMODEL 3

struct DarkDBLinkPhysAttach {
    Coord offset;
};

struct DarkDBLinkPlayerFactory {
    sint32 unknown;
};

struct DarkDBLinkProjectile {
    sint32 order;
    sint32 setting;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBLinkQuestbit;
#else
#define DarkDBLinkQuestbit DarkLabel
#endif

struct DarkDBLinkReactParam {
    sint32 receptron;
};

/* The object named "This Sensor" is used for self-targetting.
 * "This Source" is used for a self-source.
 */
struct DarkDBLinkReceptron {
    sint32 ordinal;
    float minintensity;
    float maxintensity;
    uint32 flags;
    char effect[32];
    sint32 target;
    sint32 agent;
    union {
        struct /* Amplify */
        {
            float multiply;
            float add;
        } amplify;
        struct /* WeakPoint */
        {
            float multiply;
            float add;
            Coord offset;
            float radius;
        } weakpoint;
        struct /* stimulate */
        {
            char name[16];
            float add;
            float multiply;
        } stimulate;
        struct /* stim_script_msg */
        {
            sint32 refcount;
            uint32 flags;
        } scriptmsg;
        struct /* tweq_control */
        {
            uint32 type;
            uint32 action;
            uint32 anim;
        } tweq;
#pragma pack(push, 2)
        struct /* SetQvar */
        {
            char name[28];
            uint16 operation;
            sint16 value;
        } qvar;
#pragma pack(pop)
        struct /* impact_result */
        {
            uint32 result;
        } impact;
        struct /* permeate */
        {
            float coefficient;
            float magnitude;
        } permeate;
        struct /* add_prop rem_prop */
        {
            char name[16];
        } property;
        struct /* create_obj move_obj */
        {
            Coord position;
            float heading;
            float pitch;
            float bank;
        } object;
        struct /* damage spoofdamage */
        {
            sint32 add;
            sint32 type;
            float multiply;
            Boolean stimtype;
        } damage;
        struct /* set_model */
        {
            char name[16];
        } changemodel;
        struct /* AwareFilter */
        {
            uint32 abortflags;
        } awarenessfilter;
        struct /* Freeze */
        {
            sint32 duration;
        } freeze;
        struct /* EnvSound */
        {
            char tags[32];
        } sound;
        struct /* */
        {
            float increment;
        } radiate;
        struct /* Stun */
        {
            sint32 duration;
            char tags[28];
        } stun;
        /* Abort */
        /* add_metaprop rem_metaprop */
        /* destroy obj frob_obj slay */
        /* clone_props */
        /* Knockout */
        /* weapon_hit */
        /* weapon_block */
        /* light_off light_on */
        /* toxin */
        char zero[32];
    } params;
};
#define RECEPTRON_NOMIN 1
#define RECEPTRON_NOMAX 2
#define AWARENESSFILTER_NOSEE 1
#define AWARENESSFILTER_NOHEAR 2
#define AWARENESSFILTER_STUNNED 4
#define AWARENESSFILTER_NOTAI 8
#define IMPACT_BOUNCE 2
#define IMPACT_STICK 3
#define IMPACT_SLAY 4

/* Same as FlowContact */
struct DarkDBLinkScriptContact {
    sint32 unknown[4];
};

#ifdef __cplusplus
typedef DarkLabel DarkDBLinkScriptParams;
#else
#define DarkDBLinkScriptParams DarkLabel
#endif

struct DarkDBLinkStimSensor {
    sint32 sensorcount;
};

struct DarkDBLinkTPath {
    float speed;
    sint32 pausetime;
    Boolean pathlimit;
    sint32 zero;
};

struct DarkDBLinkVoiceOver {
    uint32 flags;
};
#define VOEVENT_HILIGHT 1
#define VOEVENT_PICKUP 2

struct DarkDBLinkWeaponOffset {
    uint32 modes;
    Coord transform[4];
    uint32 joint;
};
#define WEAPONMODE_BLOCK 1
#define WEAPONMODE_MELEE 2
#define WEAPONMODE_RANGED 4
#define WEAPONMODE_IDLE 8

struct DarkDBLinkWHBlock {
    sint32 unknown;
};

#ifdef __cplusplus
} // namespace Dark
#endif

#endif
