/******************************************************************************
 *    DarkDBPropDefs.h
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

#ifndef DARK_DARKDBPROPDEFS_H
#define DARK_DARKDBPROPDEFS_H

#ifdef __cplusplus
namespace Dark {
#endif

struct DarkDBPropAcoustics {
    uint32 eax;
    sint32 dampening;
    sint32 height;
};

struct DarkDBPropAdvPickSound {
    struct {
        char pick1[24];
        char pick2[24];
        char pick3[24];
    } stage[9];
};

struct DarkDBPropAdvPickState {
    Boolean enable;
    uint32 source1;
    uint32 source2;
    uint32 source3;
    sint32 time[9];
};

struct DarkDBPropAdvPickTrans {
    struct {
        sint32 pick1;
        sint32 pick2;
        sint32 pick3;
    } stage[9];
};

struct DarkDBPropAI {
    char behaviorset[32];
};

struct DarkDBPropAIAggression {
    uint32 rating;
};

struct DarkDBPropAIAlertCap {
    uint32 maxlevel;
    uint32 minlevel;
    uint32 minrelax;
};

struct DarkDBPropAIAlertness {
    uint32 level;
    uint32 peak;
};
#define PROPVERSION_AIALERTNESS 0x00020008

struct DarkDBPropAIAlrtRsp {
    uint32 level;
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[10];
};

/* Thief1/G and SShock2. Is essentially compatible. */
struct DarkDBProp1AIAlrtRsp {
    uint32 level;
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[2];
};

struct DarkDBPropAIAlSnMultiplier {
    struct {
        float hangle;
        float vangle;
        float range;
        float knowledge;
    } level[4];
    float combat;
};

struct DarkDBPropAIAptitude {
    uint32 rating;
};

struct DarkDBPropAIAwareCapacitor {
    float discharge[3];
    sint32 unknown;
};

struct DarkDBPropAIAwrDelay {
    sint32 time_to_two;
    sint32 time_to_three;
    sint32 retrigger_two;
    sint32 retrigger_three;
    sint32 ignore_range;
};

struct DarkDBPropAIBcstSettings {
    Boolean disable;
    struct {
        uint32 schema;
        uint32 type;
        char custom[16];
        char tags[64];
    } state[8];
};
#define BROADCAST_NONE 0
#define BROADCAST_SLEEPING 1
#define BROADCAST_ALERT_0 2
#define BROADCAST_ALERT_1 3
#define BROADCAST_ALERT_2 4
#define BROADCAST_ALERT_3 5
#define BROADCAST_ALERT_TO1 6
#define BROADCAST_ALERT_TO2 7
#define BROADCAST_ALERT_TO3 8
#define BROADCAST_SPOT_PLAYER 9
#define BROADCAST_ALERT_DOWN 10
#define BROADCAST_LOST_CONTACT 11
#define BROADCAST_CHARGE 12
#define BROADCAST_SHOOT 13
#define BROADCAST_FLEE 14
#define BROADCAST_FRIEND 15
#define BROADCAST_ALARM 16
#define BROADCAST_ATTACK 17
#define BROADCAST_ATTACK_HIT 18
#define BROADCAST_BLOCK 19
#define BROADCAST_BLOCK_ATTEMPT 20
#define BROADCAST_BLOCKED 21
#define BROADCAST_HIT_NO_DAMAGE 22
#define BROADCAST_HIT_HIGH 23
#define BROADCAST_HIT_LOW 24
#define BROADCAST_AMBUSH 25
#define BROADCAST_DIE_LOUD 26
#define BROADCAST_DIE_SOFT 27
#define BROADCAST_BODY 28
#define BROADCAST_MISSING 29
#define BROADCAST_SECURITY 30
#define BROADCAST_ANOMALY_SMALL 31
#define BROADCAST_ANOMALY_LARGE 32
#define BROADCAST1_INTRUDER 33
#define BROADCAST1_BODY_SEEN 34
#define BROADCAST1_MISSING_SEEN 35
#define BROADCAST1_ANOMALY_SEEN 36
#define BROADCAST1_FRUSTRATED 37
#define BROADCAST2_ROBOT 33
#define BROADCAST2_INTRUDER 34
#define BROADCAST2_BODY_SEEN 35
#define BROADCAST2_MISSING_SEEN 36
#define BROADCAST2_ANOMALY_SEEN 37
#define BROADCAST2_ROBOT_SEEN 38
#define BROADCAST2_FRUSTRATED 39
#define BROADCAST_NULL 40
#define BROADCAST_TYPE_NORMAL 0
#define BROADCAST_TYPE_NONE 1
#define BROADCAST_TYPE_CUSTOM 2

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIBlkVision;
#else
#define DarkDBPropAIBlkVision DarkBoolean
#endif

struct DarkDBPropAIBodyRsp {
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[10];
};

/* Thief1/G and SShock2. Is essentially compatible. */
struct DarkDBProp1AIBodyRsp {
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[2];
};

struct DarkDBPropAICamera {
    float minangle;
    float maxangle;
    float scanspeed;
};

struct DarkDBPropAICbtRsp {
    sint32 unknown[5];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[10];
};

/* Thief1/G and SShock2. Is essentially compatible. */
struct DarkDBProp1AICbtRsp {
    sint32 unknown[5];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[2];
};

struct DarkDBPropAICbtTiming {
    sint32 createtime;
    sint32 droptime;
    sint32 mindelay;
    sint32 maxdelay;
};

/* XXX: Fix me
 * The extra steps aren't likely used,
 * But you should set the actor to CONV_UNUSED anyway.
 * Unused actors are -1. Else, one less than the actor number.
 */
struct DarkDBPropAIConversation {
    char unk1[16];
    uint32 abort_level;
    uint32 abort_priority;
    char unk2[12];
    struct {
        sint32 actor;
        Boolean noblock;
        DarkPScriptCommand command;
    } step[6];
    struct {
        sint32 actor;
        Boolean noblock;
        DarkPScriptCommand command;
    } extra[66];
};
#define CONV_UNUSED -1;

struct DarkDBPropAICoverPt {
    sint32 value;
    float decayspeed;
    Boolean canduck;
};
#define PROPVERSION_AICOVERPT 0x0001000C

struct DarkDBPropAIDefensiveness {
    uint32 rating;
};

struct DarkDBPropAIDevice {
    sint32 joint;
    float inactive;
    float active;
    float speed;
    sint32 rotation_joint;
    float facing;
    Boolean rotate_on;
};

struct DarkDBProp1AIDevice {
    sint32 joint;
    float inactive;
    float active;
    float speed;
    sint32 rotation_joint;
    float facing;
};

struct DarkDBPropAIDodginess {
    uint32 rating;
};

struct DarkDBPropAIEfficiency {
    Boolean enabled;
    struct {
        float entry;
        float entry_height;
        float exit;
        float exit_height;
    } super, normal;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIFidget;
#else
#define DarkDBPropAIFidget DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIFiresThrough;
#else
#define DarkDBPropAIFiresThrough DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIFleeAwr;
#else
#define DarkDBPropAIFleeAwr DarkBoolean
#endif

struct DarkDBPropAIFleeCondition {
    uint32 condition;
    sint32 hp_percent;
    sint32 friends;
};
#define FLEE_NEVER 0
#define FLEE_ALERT1 1
#define FLEE_ALERT2 2
#define FLEE_ALERT3 3
#define FLEE_DAMAGE 4
#define FLEE_THREAT 5
#define FLEE_FRUSTRATED 6

struct DarkDBPropAIFleePoint {
    sint32 value;
};

struct DarkDBPropAIFreeKnowledge {
    sint32 value;
};

struct DarkDBPropAIFrozen {
    sint32 start;
    sint32 duration;
};

struct DarkDBPropAIFrustrated {
    sint32 source;
    sint32 dest;
    sint32 unknown;
};

struct DarkDBPropAIGrubCombat {
    float leapdistance;
    float bitedistance;
    char stimulus[32];
    float intensity;
    float horizontalspeed;
    float verticalspeed;
    float minleaptime;
    float maxleaptime;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropAIGun;
#else
#define DarkDBPropAIGun DarkLabel
#endif

#pragma pack(push, 2)
struct DarkDBPropAIGunDescription {
    float maxrange;
    Coord fireoffset;
    sint32 startlag;
    sint32 burstlag;
    sint32 endlag;
    sint16 aimerror;
};
#pragma pack(pop)
#define PROPVERSION_AIGUNDESCRIPTION 0x0001001E

struct DarkDBPropAIHearing {
    uint32 rating;
};

/* only first three are used */
struct DarkDBPropAIIdleDirections {
    sint32 mintime;
    sint32 maxtime;
    struct {
        sint32 facing;
        sint32 weight;
    } direction[4];
};

struct DarkDBPropAIIdleOrigin {
    Coord position;
    float facing;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIIdlRetOrg;
#else
#define DarkDBPropAIIdlRetOrg DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIIgCamera;
#else
#define DarkDBPropAIIgCamera DarkBoolean
#endif

struct DarkDBPropAIInfDly {
    sint32 delay;
};

struct DarkDBPropAIInfDst {
    sint32 distance;
};

struct DarkDBPropAIInfExpiration {
    sint32 time;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIInfFrm;
#else
#define DarkDBPropAIInfFrm DarkBoolean
#endif

struct DarkDBPropAIInfMxPasses {
    sint32 passes;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIInfNow;
#else
#define DarkDBPropAIInfNow DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIInfOtr;
#else
#define DarkDBPropAIInfOtr DarkBoolean
#endif

struct DarkDBPropAIInfRsp {
    Boolean disable;
    Boolean nodefault;
    sint32 unknown[5];
    struct DarkPScriptCommand step[2];
    struct DarkPScriptCommand extra[6];
};

/* Investigation Style */
struct DarkDBPropAIInvKnd {
    uint32 type;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIIsBig;
#else
#define DarkDBPropAIIsBig DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIIsSmall;
#else
#define DarkDBPropAIIsSmall DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIIsProxy;
#else
#define DarkDBPropAIIsProxy DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAILaunchVisible;
#else
#define DarkDBPropAILaunchVisible DarkBoolean
#endif

struct DarkDBPropAIMode {
    uint32 mode;
};
#define AIMODE_ASLEEP 0
#define AIMODE_SUPEREFFICIENT 1
#define AIMODE_EFFICIENT 2
#define AIMODE_NORMAL 3
#define AIMODE_COMBAT 4
#define AIMODE_DEAD 5
#define AIMODE_KNOCKEDOUT AIMODE_DEAD

#ifdef __cplusplus
typedef DarkString DarkDBPropAIMotTags;
#else
#define DarkDBPropAIMotTags DarkString
#endif

struct DarkDBPropAIMoveSpeed {
    float speed;
};

struct DarkDBPropAIMoveZOffset {
    float height;
};

struct DarkDBPropAINCDmgRsp {
    sint32 woundthreshold;
    sint32 severethreshold;
    sint32 responsechance;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAINCDmRsp;
#else
#define DarkDBPropAINCDmRsp DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAINGOBB;
#else
#define DarkDBPropAINGOBB DarkBoolean
#endif
#define DarkDBPropAIExactOBB DarkDBPropAINGOBB

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAINoGhost;
#else
#define DarkDBPropAINoGhost DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAINoHandoff;
#else
#define DarkDBPropAINoHandoff DarkBoolean
#endif

struct DarkDBPropAINonHstile {
    uint32 condition;
};
#define AINONHOSTILE_NEVER 0
#define AINONHOSTILE_TOPLAYER 1
#define AINONHOSTILE_DAMAGE_TOPLAYER 2
#define AINONHOSTILE_THREAT_TOPLAYER 3
#define AINONHOSTILE_DAMAGE 4
#define AINONHOSTILE_THREAT 5
#define AINONHOSTILE_ALWAYS 6

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAINoticeDmg;
#else
#define DarkDBPropAINoticeDmg DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAINotMelee;
#else
#define DarkDBPropAINotMelee DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAINtcBody;
#else
#define DarkDBPropAINtcBody DarkBoolean
#endif

struct DarkDBPropAIObjAvoid {
    sint32 zero;
    uint32 flags;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIObjPathable;
#else
#define DarkDBPropAIObjPathable DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIOnlyPlayer;
#else
#define DarkDBPropAIOnlyPlayer DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIPatrol;
#else
#define DarkDBPropAIPatrol DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIPtrlRnd;
#else
#define DarkDBPropAIPtrlRnd DarkBoolean
#endif

struct DarkDBProp1AIRCProp {
    sint32 mindistance;
    sint32 idealdistance;
    float firingdelay;
    uint32 coverdesire;
    float decayspeed;
    uint32 firewhilemoving;
};
#define PROPVERSION1_AIRCPROP 0x00010018
struct DarkDBPropAIRCProp {
    sint32 mindistance;
    sint32 idealdistance;
    float firingdelay;
    uint32 coverdesire;
    float decayspeed;
    uint32 firewhenmoving;
    Boolean containprojectile;
};
#define PROPVERSION2_AIRCPROP 0x0001001C

struct DarkDBPropAIRCRanges {
    float mindistance;
    float shortdistance;
    float longdistance;
    float maxdistance;
};
#define PROPVERSION2_AIRCRANGES 0x00010010

struct DarkDBPropAIRCWndSnd {
    sint32 woundthreshold;
    sint32 severethreshold;
    sint32 responsechance;
};

struct DarkDBPropAIRCWoundMotion {
    sint32 woundthreshold;
    sint32 severethreshold;
    sint32 responsechance;
};

struct DarkDBPropAIRngApplicable {
    uint32 idle;
    uint32 zero1;
    uint32 close;
    uint32 backup;
    uint32 wound;
    uint32 vantage;
    uint32 left;
    uint32 right;
    uint32 zero2;
};
#define PROPVERSION_AIRNGAPPLICABLE 0x00010024

struct DarkDBPropAIRngFlee {
    uint32 veryshortpriority;
    uint32 shortpriority;
    float anglerange;
    sint32 numpoints;
    float distance;
    float clearance;
};
#define PROPVERSION_AIRNGFLEE 0x00010018

struct DarkDBPropAIRngShoot {
    uint32 veryshortpriority;
    uint32 shortpriority;
    uint32 idealpriority;
    uint32 longpriority;
    uint32 verylongpriority;
    Boolean confirmrange;
    Boolean confirmlof;
    sint32 rotationspeed;
    Coord launchoffset;
};
#define PROPVERSION_AIRNGSHOOT 0x0001002C

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAISaveConversation;
#else
#define DarkDBPropAISaveConversation DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAISeesPrj;
#else
#define DarkDBPropAISeesPrj DarkBoolean
#endif

struct DarkDBPropAISigRsp {
    char signal[32];
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[10];
};

struct DarkDBProp1AISigRsp {
    char signal[32];
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[2];
};

struct DarkDBPropAISloth {
    uint32 rating;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropAISndTags;
#else
#define DarkDBPropAISndTags DarkString
#endif

struct DarkDBPropAISndType {
    uint32 type;
    char signal[32];
    sint32 zero;
};
#define PROPVERSION_AISNDTYPE 0x00020028
#define AISOUNDTYPE_NONE 0
#define AISOUNDTYPE_INFORM 1
#define AISOUNDTYPE_MINOR 2
#define AISOUNDTYPE_MAJOR 3
#define AISOUNDTYPE_NONCOMBAT 4
#define AISOUNDTYPE_COMBAT 5

#ifdef __cplusplus
typedef DarkString DarkDBPropAIStandTags;
#else
#define DarkDBPropAIStandTags DarkString
#endif

/* the fields don't have to be 0 and 1,
 * that's just how Dromed labels them.
 */
struct DarkDBPropAISurprise {
    float zero;
    float one;
    float radius;
};

/* No Thief1 version of this one. */
struct DarkDBPropAISuspRsp {
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[10];
};

struct DarkDBPropAISwarm {
    float closedistance;
    float backoffdistance;
};

struct DarkDBPropAITeam {
    uint32 team;
};

struct DarkDBPropAIThrtRsp {
    uint32 threat;
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[10];
};

struct DarkDBProp1AIThrtRsp {
    uint32 threat;
    uint32 priority;
    sint32 unknown[4];
    struct DarkPScriptCommand step[6];
    struct DarkPScriptCommand extra[2];
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAITrackMedium;
#else
#define DarkDBPropAITrackMedium DarkBoolean
#endif

struct DarkDBPropAITurnRate {
    float speed;
};

struct DarkDBPropAITurret {
    float fireepsilon;
    sint32 firepause;
    float pitchepsilon;
    float range;
};

struct DarkDBProp1AITurret {
    float fireepsilon;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIUsesDoors;
#else
#define DarkDBPropAIUsesDoors DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAIUseWater;
#else
#define DarkDBPropAIUseWater DarkBoolean
#endif

struct DarkDBPropAIVantagePt {
    sint32 value;
    float decayspeed;
};
#define PROPVERSION_AIVANTAGEPT 0x00010008

struct DarkDBPropAIVerbosity {
    uint32 rating;
};

/* cornerleanmod is not an integer, as Dromed seems to think */
struct DarkDBPropAIVisCtrl {
    sint32 lowlight;
    sint32 midlight;
    sint32 highlight;
    float slow;
    float fast;
    sint32 slowmod;
    sint32 normalmod;
    sint32 fastmod;
    float walldist;
    sint32 crouchmod;
    sint32 wallmod;
    float cornerleanmod;
};

struct DarkDBPropAIVisDescription {
    sint32 unknown[8];
    struct {
        uint32 flags;
        sint32 angle;
        sint32 zangle;
        sint32 range;
        sint32 acuity;
    } viscone[10];
    float zoffset;
};
#define AIVISION_ACTIVE 0x001
#define AIVISION_NOALERT0 0x002
#define AIVISION_NOALERT1 0x004
#define AIVISION_NOALERT2 0x008
#define AIVISION_NOALERT3 0x010
#define AIVISION_PERIPHERAL 0x020
#define AIVISION_OMNI 0x040
#define AIVISION_NIGHT 0x080
#define AIVISION_BEHIND 0x100

struct DarkDBPropAIVisibility {
    sint32 level;
    sint32 light;
    sint32 movement;
    sint32 exposure;
    sint32 lastupdate;
};

struct DarkDBPropAIVision {
    uint32 rating;
};

struct DarkDBPropAIVisJoint {
    uint32 joint;
};

struct DarkDBPropAIVisModifier {
    float factor[6];
};
#define PROPVERSION_AIVISMODIFIER 0x00010018

struct DarkDBPropAIVisType {
    uint32 type;
};

struct DarkDBPropAIWander {
    float distance;
};

struct DarkDBPropAIWtchPnt {
    sint32 unknown[15];
    uint32 trigger;
    uint32 awareness;
    uint32 visibility;
    sint32 unknown2;
    uint32 killcondition;
    Boolean killlikelinks;
    Boolean onceonly;
    sint32 reusetime;
    sint32 resettime;
    uint32 minalertness;
    uint32 maxalertness;
    uint32 priority;
    sint32 radius;
    sint32 height;
    struct DarkPScriptCommand step[7];
    struct DarkPScriptCommand extra[1];
    /*struct DarkPScriptCommand	extra2[8];	Even more extra in Dark2, leave it
     * up to the app to fill out */
};

/* XXX: What, exactly, is the value? Milliseconds? */
struct DarkDBPropAirSupply {
    sint32 value;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAlarm;
#else
#define DarkDBPropAlarm DarkBoolean
#endif

struct DarkDBPropAlchemy {
    float value;
};

struct DarkDBPropAltLink {
    uint32 joint;
    struct {
        sint32 x, y, z;
    } rotation;
    Coord offset;
};

struct DarkDBPropAmbient {
    char name[64];
    sint32 volume;
};

struct DarkDBPropAmbientHacked {
    sint32 radius;
    sint32 volume;
    uint32 flags;
    char schema[16];
    char aux1[16];
    char aux2[16];
};
#define AMBIENT_ENVIRONMENTAL 0x001
#define AMBIENT_NOSHARPCURVE 0x002
#define AMBIENT_TURNEDOFF 0x004
#define AMBIENT_REMOVEPROP 0x008
#define AMBIENT_MUSIC 0x010
#define AMBIENT_SYNCH 0x020
#define AMBIENT_NOFADE 0x040
#define AMBIENT_DESTROYOBJ 0x080
#define AMBIENT_DOAUTOOFF 0x100

struct DarkDBPropAngleLimit {
    float start;
    float end;
};

#pragma pack(push, 2)
struct DarkDBPropAnimLight {
    sint32 unk1;
    Coord offset;
    sint32 unk2;
    Boolean quadlit;
    sint16 unk3;
    uint16 mode;
    sint32 brightentime;
    sint32 dimtime;
    float minbrightness;
    float maxbrightness;
    sint32 unk4;
    Boolean rising;
    sint32 countdown;
    Boolean inactive;
    float radius;
    sint32 unk5[2];
    float innerradius;
};

struct DarkDBPropAnim1Light {
    sint32 unk1;
    Coord offset;
    sint32 unk2[2];
    sint16 unk3;
    uint16 mode;
    sint32 brightentime;
    sint32 dimtime;
    float minbrightness;
    float maxbrightness;
    sint32 unk4;
    Boolean rising;
    sint32 countdown;
    Boolean inactive;
    float radius;
    sint32 unk5;
};
#pragma pack(pop)

struct DarkDBPropGunAnim {
    uint32 joint;
    uint32 flags;
    struct {
        float target;
        float rate;
    } mode[2];
};
#define DarkDBPropAnimPre DarkDBPropGunAnim
#define DarkDBPropAnimPost DarkDBPropGunAnim

struct DarkDBPropAnimTexture {
    sint32 speed;
    uint32 flags;
};
#define ANIMTEX_WRAP 0x1
#define ANIMTEX_RANDING 0x2
#define ANIMTEX_REVERSE 0x4
#define ANIMTEX_PORTAL 0x8

#ifdef __cplusplus
typedef DarkLabel DarkDBPropApparID;
#else
#define DarkDBPropApparID DarkLabel
#endif

struct DarkDBPropArmor {
    float toxic;
    float radiation;
    float combat;
};

struct DarkDBPropARSrcScale {
    float scale;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBPropAudioLog;
#else
#define DarkDBPropAudioLog DarkLabel
#endif

struct DarkDBPropAutomap {
    sint32 page;
    sint32 location;
};
#define PROPVERSION_AUTOMAP 0x00020008

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropAutoPickup;
#else
#define DarkDBPropAutoPickup DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropAutoVBR;
#else
#define DarkDBPropAutoVBR DarkLabel
#endif

struct DarkDBPropAvatarZOffset {
    float height;
};

struct DarkDBPropBaseGunDescription {
    struct {
        sint32 burst;
        sint32 clip;
        sint32 spray;
        float stimmult;
        sint32 bursttime;
        sint32 shottime;
        sint32 ammousage;
        float speedmult;
        sint32 reloadtime;
        sint32 zero;
    } setting[3];
    /* SShock2 doesn't have these */
    sint32 numsettings;
    float zoomfactor[3];
};
#define PROPVERSION_BASEGUNDESCRIPTION 0x00010088

struct DarkDBPropStatsDesc {
    sint32 strength;
    sint32 endurance;
    sint32 psi;
    sint32 agility;
    sint32 cyber;
};
#define DarkDBPropArmrStatsDesc DarkDBPropStatsDesc
#define DarkDBPropBaseStatsDesc DarkDBPropStatsDesc
#define DarkDBPropDrugStatsDesc DarkDBPropStatsDesc
#define DarkDBPropImplStatsDesc DarkDBPropStatsDesc
#define DarkDBPropPsiStatsDesc DarkDBPropStatsDesc
#define DarkDBPropReqStatsDesc DarkDBPropStatsDesc
#define PROPVERSION_BASESTATS 0x00000114

struct DarkDBPropTechDesc {
    sint32 hack;
    sint32 repair;
    sint32 modify;
    sint32 maintain;
    sint32 research;
};
#define DarkDBPropBaseTechDesc DarkDBPropTechDesc
#define DarkDBPropImplTechDesc DarkDBPropTechDesc
#define DarkDBPropReqTechDesc DarkDBPropTechDesc

struct DarkDBPropBaseWeaponDesc {
    sint32 conventional;
    sint32 energy;
    sint32 heavy;
    sint32 annelid;
};

struct DarkDBPropBaseWpnDmg {
    sint32 damage;
};
#define PROPVERSION_BASEWPNDMG 0x00010004

struct DarkDBPropBashFactor {
    float value;
};

struct DarkDBPropBashParams {
    float threshold;
    float coefficient;
};
#define PROPVERSION_BASHPARAMS 0x000003E8

struct DarkDBPropBeltLink {
    uint32 joint;
    struct {
        sint32 x, y, z;
    } rotation;
    Coord offset;
};

struct DarkDBPropBitmapAnimation {
    uint32 flags;
};

struct DarkDBPropBitmapWorldSpace {
    XYCoord size;
    XYCoord feetpertile;
};
#define PROPVERSION_BITMAPWORLDSPACE 0x00010010

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropBlockFrob;
#else
#define DarkDBPropBlockFrob DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropBlood;
#else
#define DarkDBPropBlood DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropBloodCause;
#else
#define DarkDBPropBloodCause DarkBoolean
#endif

struct DarkDBPropBloodMaxDamage {
    sint32 damage;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropBloodType;
#else
#define DarkDBPropBloodType DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropBook;
#else
#define DarkDBPropBook DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropBookArt;
#else
#define DarkDBPropBookArt DarkString
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropBookData;
#else
#define DarkDBPropBookData DarkLabel
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropBorrowed;
#else
#define DarkDBPropBorrowed DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropBorrowing;
#else
#define DarkDBPropBorrowing DarkBoolean
#endif

struct DarkDBPropBreathConfig {
    sint32 maxair;
    sint32 drownfreq;
    sint32 drowndamage;
    float recoverrate;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropBumpMap;
#else
#define DarkDBPropBumpMap DarkBoolean
#endif

#pragma pack(push, 2)
struct DarkDBPropCameraObj {
    Coord offset;
    SCoord facing;
    sint16 zero;
    Boolean lockxaxis;
    Boolean lockyaxis;
    Boolean lockzaxis;
    Boolean isvisible;
};
#pragma pack(pop)

/* This was a Boolean in T1 */
struct DarkDBPropCanAttach {
    uint32 type;
};

struct DarkDBPropCannister {
    sint32 count;
    float speed;
    sint32 generations;
};

#define TWEQHALT_DESTROYOBJ 0
#define TWEQHALT_REMOVEPROP 1
#define TWEQHALT_STOPTWEQ 2
#define TWEQHALT_CONTINUE 3
#define TWEQHALT_SLAYOBJ 4
#define TWEQANIM_NOLIMIT 0x01
#define TWEQANIM_SIM 0x02
#define TWEQANIM_WRAP 0x04
#define TWEQANIM_ONEBOUNCE 0x08
#define TWEQANIM_SIMSMALLRAD 0x10
#define TWEQANIM_SIMLARGERAD 0x20
#define TWEQANIM_OFFSCREEN 0x40
#define TWEQCURVE_JITTERLOW 0x1
#define TWEQCURVE_JITTERHI 0x2
#define TWEQCURVE_JITTERMUL 0x4
#define TWEQMISC_ANCHOR 0x001
#define TWEQMISC_SCRIPTS 0x002
#define TWEQMISC_RANDOM 0x004
#define TWEQMISC_GRAV 0x008
#define TWEQMISC_ZEROVEL 0x010
#define TWEQMISC_TELLAI 0x020
#define TWEQMISC_PUSHOUT 0x040
#define TWEQMISC_NEGATIVELOGIC 0x080
#define TWEQMISC_RELATIVEVELOCITY 0x100
#define TWEQMISC_NOPHYSICS 0x200
#define TWEQMISC_ANCHORVHOT 0x400
#define TWEQMISC_HOSTONLY 0x800
#define TWEQSTATE_ON 0x01
#define TWEQSTATE_REVERSE 0x02
#define TWEQSTATE_RESYNCH 0x04
#define TWEQSTATE_GOEDGE 0x08
#define TWEQSTATE_LAPONE 0x10

#pragma pack(push, 1)
struct DarkDBPropCfgTweqBlink {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 rate;
};

struct DarkDBPropCfgTweqDelete {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 rate;
};

struct DarkDBPropCfgTweqEmit {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 rate;
    sint32 maxframes;
    char objectname[16];
    Coord velocity;
    Coord anglerandom;
};
struct DarkDBProp1CfgTweqEmit {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 rate;
    uint32 maxframes;
    char objectname[16];
    Coord velocity;
};
#define DarkDBPropCfgTweq2Emit DarkDBPropCfgTweqEmit
#define DarkDBPropCfgTweq3Emit DarkDBPropCfgTweqEmit
#define DarkDBPropCfgTweq4Emit DarkDBPropCfgTweqEmit
#define DarkDBPropCfgTweq5Emit DarkDBPropCfgTweqEmit

struct DarkDBPropCfgTweqJoint {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 zero;
    struct {
        uint8 unk1;
        uint8 curve;
        uint8 anim;
        uint8 zero[5];
        float rate;
        float low;
        float high;
    } joint[6];
    sint8 primary;
    uint8 unused[3];
};

struct DarkDBPropCfgTweqLock {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 zero;
    sint32 zero2[2];
    float rate;
    float low;
    float high;
    sint8 joint;
    uint8 zero3[3];
};

struct DarkDBPropCfgTweqModel {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 rate;
    char modelname[6][16];
};

struct DarkDBPropCfgTweqRotate {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 zero;
    struct {
        float rate;
        float low;
        float high;
    } x, y, z;
    sint32 primary;
};

struct DarkDBPropCfgTweqScale {
    uint8 unknown;
    uint8 curve;
    uint8 anim;
    uint8 halt;
    uint16 misc;
    uint16 zero;
    struct {
        float rate;
        float low;
        float high;
    } x, y, z;
    sint32 primary;
};
#pragma pack(pop)

struct DarkDBPropCharGenRoom {
    sint32 roomid;
};

struct DarkDBPropCharGenYear {
    sint32 year;
};

struct DarkDBPropChemNeeded {
    char item[7][64];
    sint32 time[7];
};

struct DarkDBPropClassTags {
    /* Someone please tell me what the hell 'value' is */
    sint32 value;
    char tags[252];
};

struct DarkDBPropClimbability {
    float factor;
};

struct DarkDBPropCollisionType {
    uint32 flags;
};
#define COLLTYPE_BOUNCE 0x01
#define COLLTYPE_DESTROY 0x02
#define COLLTYPE_SLAY 0x04
#define COLLTYPE_NOSOUND 0x08
#define COLLTYPE_NORESULT 0x10
#define COLLTYPE_FULLSOUND 0x20

#ifdef __cplusplus
typedef DarkLabel DarkDBPropCombineType;
#else
#define DarkDBPropCombineType DarkLabel
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropConsumeType;
#else
#define DarkDBPropConsumeType DarkString
#endif

struct DarkDBPropContainDims {
    sint32 width;
    sint32 height;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropContainInherit;
#else
#define DarkDBPropContainInherit DarkBoolean
#endif
#define PROPVERSION_CONTAININHERIT 0x00010004

struct DarkDBPropConveyorVelocity {
    float x, y, z;
};

struct DarkDBPropCorona {
    sint32 unknown1;
    Coord position;
    float alpha;
    float maxsize;
    float maxdist;
    float minsize;
    char bitmap[16];
    sint32 unknown5;
};
#define PROPVERSION_CORONA 0x00010034

struct DarkDBPropCreature {
    uint32 type;
};
#define PROPVERSION_CREATURE 0x00010008
#define CREATURETYPE_HUMANOID 0
#define CREATURETYPE_PLAYERARM 1
#define CREATURETYPE_PLAYERBOWARM 2
#define CREATURETYPE_BURRICK 3
#define CREATURETYPE_SPIDER 4
#define CREATURETYPE_BUGBEAST 5
#define CREATURETYPE_CRAYMAN 6
#define CREATURETYPE_CONSTANTINE 7
#define CREATURETYPE_APPARITION 8
#define CREATURETYPE_SWEEL 9
#define CREATURETYPE_ROPE 10
#define CREATURETYPE_ZOMBIE 11
#define CREATURETYPE_SMALLSPIDER 12
#define CREATURETYPE_FROG 13
#define CREATURETYPE_CUTTY 14
#define CREATURETYPE_AVATAR 15
#define CREATURETYPE_ROBOT 16
#define CREATURETYPE_SMALLROBOT 17
#define CREATURETYPE_SPIDERBOT 18

struct DarkDBPropCretPose {
    uint32 type;
    char name[80];
    float fraction;
    float scale;
    Boolean isballistic;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBPropCSArrow;
#else
#define DarkDBPropCSArrow DarkLabel
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropCSProjectile;
#else
#define DarkDBPropCSProjectile DarkLabel
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropCSProperty;
#else
#define DarkDBPropCSProperty DarkLabel
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropCulpable;
#else
#define DarkDBPropCulpable DarkBoolean
#endif

struct DarkDBPropCurWpnDmg {
    sint32 damage;
};
#define PROPVERSION_CURWPNDMG 0x0x00010004

struct DarkDBPropDAIFrogExplodeRange {
    float distance;
};

struct DarkDBPropDarkStats {
    uint32 flags;
};
#define DARKSTAT_INNOCENT 0x01
#define DARKSTAT_ENEMY 0x02
#define DARKSTAT_HIDDEN 0x04
#define DARKSTAT_FOUNDBODY 0x08
#define DARKSTAT_ROBOT 0x10

struct DarkDBPropDeathStage {
    sint32 value;
};

struct DarkDBPropDelayTime {
    float seconds;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropDesignNote;
#else
#define DarkDBPropDesignNote DarkString
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropDestLevel;
#else
#define DarkDBPropDestLevel DarkLabel
#endif

struct DarkDBPropDestLoc {
    sint32 destination;
};

struct DarkDBPropDiffCloseOpenDoor {
    uint32 levels;
};

struct DarkDBPropDiffDestroy {
    uint32 levels;
};

struct DarkDBPropDiffLock {
    uint32 levels;
};

struct DarkDBPropDiffPermit {
    uint32 levels;
};

struct DarkDBPropDiffScript {
    uint32 levels;
};

struct DarkDBPropDiffTurnOnOff {
    uint32 levels;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropDistinctAvatar;
#else
#define DarkDBPropDistinctAvatar DarkBoolean
#endif

struct DarkDBPropDonorType {
    sint32 value;
};
#define DONOR_METAPROPERTY 1

#ifdef __cplusplus
typedef DarkLabel DarkDBPropDoorCloseSound;
#else
#define DarkDBPropDoorCloseSound DarkLabel
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropDoorOpenSound;
#else
#define DarkDBPropDoorOpenSound DarkLabel
#endif

struct DarkDBPropDoorTimer {
    sint32 seconds;
};
#define PROPVERSION_DOORTIMER 0x00010004

struct DarkDBPropDrainAmt {
    float amount;
};

struct DarkDBPropDrainRate {
    float rate;
};

struct DarkDBPropEcology {
    float time;
    sint32 minvalue[3];
    sint32 maxvalue[3];
    float recovery[3];
    sint32 randomize[3];
};

struct DarkDBPropEcoState {
    sint32 mode;
};

struct DarkDBPropEcoType {
    sint32 type;
};

struct DarkDBPropElasticity {
    float elasticity;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropElevAble;
#else
#define DarkDBPropElevAble DarkBoolean
#endif

struct DarkDBPropElevOffset {
    float x, y, z;
};

struct DarkDBPropEnergy {
    float storedenergy;
};

struct DarkDBPropExP {
    sint32 value;
};

struct DarkDBPropExtraLight {
    float factor;
    Boolean isadditive;
};
#define PROPVERSION_EXTRALIGHT 0x00010008

struct DarkDBPropFabCost {
    sint32 cost;
};

struct DarkDBPropFabricate {
    sint32 quantity;
};
#define PROPVERSION_FABRICATE 0x00010004

struct DarkDBPropFace {
    char original[16];
    char texture[5][16];
    char extra[8][16];
};
#define FACEVISAGE_NEUTRAL 0
#define FACEVISAGE_SMILE 1
#define FACEVISAGE_WINCE 2
#define FACEVISAGE_SURPRISE 3
#define FACEVISAGE_STUNNED 4

/* Not sure if "face" in this context is a verb or a noun. */
#ifdef __cplusplus
typedef DarkBoolean DarkDBPropFacePosition;
#else
#define DarkDBPropFacePosition DarkBoolean
#endif
#define PROPVERSION_FACEPOSITION 0x00010004

struct DarkDBPropFaceState {
    Boolean istalking;
    sint32 unknown[3];
    uint32 currentvisage;
    sint32 priority;
    sint32 endtime;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBPropFailSound;
#else
#define DarkDBPropFailSound DarkLabel
#endif

struct DarkDBPropFirer {
    sint32 object;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropFixture;
#else
#define DarkDBPropFixture DarkBoolean
#endif

struct DarkDBPropFlowColor {
    sint32 index;
};

struct DarkDBPropFlowGroup {
    sint32 group;
};

#pragma pack(push, 1)
struct DarkDBPropFrameAniConfig {
    float fps;
    uint8 oneshot;
    uint8 bounce;
    uint8 limit;
    uint8 killfinish;
};
#pragma pack(pop)

struct DarkDBPropFrameAniState {
    sint32 unknown1;
    sint32 unknown2;
    sint32 current;
    sint32 unknown3;
};

struct DarkDBPropFriction {
    float friction;
};

struct DarkDBPropFrobHandler {
    uint32 type;
};

struct DarkDBPropFrobInfo {
    uint32 world;
    uint32 inventory;
    uint32 tool;
    uint32 zero;
};
#define FROBINFO_MOVE 0x001
#define FROBINFO_SCRIPT 0x002
#define FROBINFO_DELETE 0x004
#define FROBINFO_IGNORE 0x008
#define FROBINFO_FOCUSSCRIPT 0x010
#define FROBINFO_TOOLCURSOR 0x020
#define FROBINFO_USEAMMO 0x040
#define FROBINFO_DEFAULT 0x080
#define FROBINFO_DESELECT 0x100

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropFrobLocally;
#else
#define DarkDBPropFrobLocally DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropFrobSound;
#else
#define DarkDBPropFrobSound DarkLabel
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropFromBriefcase;
#else
#define DarkDBPropFromBriefcase DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropFungus;
#else
#define DarkDBPropFungus DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropGameDescription;
#else
#define DarkDBPropGameDescription DarkString
#endif
#define PROPVERSION_GAMEDESCRIPTION 0x00010011

#ifdef __cplusplus
typedef DarkString DarkDBPropGameName;
#else
#define DarkDBPropGameName DarkString
#endif
#define PROPVERSION_GAMENAME 0x00010011

#ifdef __cplusplus
typedef DarkLabel DarkDBPropGuarLoot;
#else
#define DarkDBPropGuarLoot DarkLabel
#endif

#pragma pack(push, 2)
struct DarkDBPropGunKick {
    struct {
        float preshot;
        uint16 kickpitch;
        uint16 kickpitchmax;
        uint16 kickheading;
        uint16 kickreturnangle;
        float kickback;
        float kickbackmax;
        float kickreturn;
        uint16 joltpitch;
        uint16 joltheading;
        float joltback;
    } setting[3];
};
#pragma pack(pop)

struct DarkDBPropGunReliability {
    float minbreakage;
    float maxbreakage;
    float degraderate;
    float breakthreshold;
};

struct DarkDBPropGunState {
    sint32 ammo;
    float condition;
    sint32 setting;
    sint32 mods;
    float silencing;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropHackText;
#else
#define DarkDBPropHackText DarkString
#endif

struct DarkDBPropHackKey {
    sint32 id;
};

struct DarkDBPropHackLock {
    sint32 id;
};

struct DarkDBPropHackTime {
    sint32 duration;
};

struct DarkDBPropHackVisi {
    float visibility;
};

struct DarkDBPropHandoffNumber {
    sint32 value;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropHasBrush;
#else
#define DarkDBPropHasBrush DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropHasRefs;
#else
#define DarkDBPropHasRefs DarkBoolean
#endif

struct DarkDBPropHeartbeat {
    sint32 duration;
};

struct DarkDBPropHeatDisks {
    Coord start;
    Coord end;
    float startradius;
    float endradius;
    float bottomjitter;
    float topjitter;
    float height;
    sint32 blobs;
    sint32 maxdisks;
};

struct DarkDBPropHitPoints {
    sint32 hp;
};

struct DarkDBPropHoming {
    uint32 targettype;
    float distancefilter;
    float headingfilter;
    sint32 maxturn;
    sint32 updatetime;
};

/* XXX: Fix me */
struct DarkDBPropHTHAudioResponse {
    float x, y, z;
};

/* XXX: Fix me */
struct DarkDBPropHTHCombatDistances {
    float x, y, z;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropHTHGruntAlways;
#else
#define DarkDBPropHTHGruntAlways DarkBoolean
#endif

/* XXX: Fix me */
struct DarkDBPropHTHModeOverride {
    sint32 value;
};

/* XXX: Fix me */
struct DarkDBPropHTHMotionResponse {
    float x, y, z;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropHUDSelect;
#else
#define DarkDBPropHUDSelect DarkBoolean
#endif

struct DarkDBPropHUDTime {
    sint32 duration;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropHUDUse;
#else
#define DarkDBPropHUDUse DarkString
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropImmobile;
#else
#define DarkDBPropImmobile DarkBoolean
#endif

struct DarkDBPropImplantDesc {
    uint32 type;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropInitted;
#else
#define DarkDBPropInitted DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropInvBeingTaken;
#else
#define DarkDBPropInvBeingTaken DarkBoolean
#endif
#define PROPVERSION_INVBEINGTAKEN 0x00010004

#ifdef __cplusplus
typedef DarkString DarkDBPropInvCursor;
#else
#define DarkDBPropInvCursor DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropInvCycleOrder;
#else
#define DarkDBPropInvCycleOrder DarkString
#endif
#define PROPVERSION_INVCYCLEORDER 0x00010011

struct DarkDBPropInvDims {
    sint32 width;
    sint32 height;
};

struct DarkDBPropInvisible {
    sint32 visibility;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBPropInvLimbModel;
#else
#define DarkDBPropInvLimbModel DarkLabel
#endif

struct DarkDBPropInvRendType {
    uint32 type;
    char name[16];
};
#define PROPVERSION_INVRENDTYPE 0x00010014

struct DarkDBPropInvType {
    uint32 type;
};
#define PROPVERSION_INVTYPE 0x00010004

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropItemStore;
#else
#define DarkDBPropItemStore DarkBoolean
#endif
#define PROPVERSION_ITEMSTORE 0x00010004

struct DarkDBPropJointPositions {
    float position[6];
};

#pragma pack(push, 1)
struct DarkDBPropKeyDst {
    uint8 master;
    uint32 regions;
    uint8 lockid;
};

struct DarkDBPropKeySrc {
    uint8 master;
    uint32 regions;
    uint8 lockid;
};

struct DarkDBProp1KeyDst {
    uint8 master;
    uint16 regions;
    uint8 lockid;
};

struct DarkDBProp1KeySrc {
    uint8 master;
    uint16 regions;
    uint8 lockid;
};
#pragma pack(pop)

struct DarkDBPropKeypadCode {
    sint32 code;
};

struct DarkDBPropLauncherMass {
    float mass;
};
#define PROPVERSION_LAUNCHERMASS 0x00010004

struct DarkDBPropLight {
    float brightness;
    Coord offset;
    float radius;
    Boolean quadlight;
    float inner;
};

struct DarkDBProp1Light {
    float brightness;
    Coord offset;
    float radius;
};

struct DarkDBPropLightColor {
    float hue;
    float saturation;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropLocalCopy;
#else
#define DarkDBPropLocalCopy DarkBoolean
#endif

/* XXX: Fix me */
struct DarkDBPropLockCnt {
    sint32 value;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropLocked;
#else
#define DarkDBPropLocked DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropLockMsg;
#else
#define DarkDBPropLockMsg DarkString
#endif

struct DarkDBPropLogs {
    uint32 emails;
    uint32 logs;
    uint32 notes;
    uint32 videos;
};

struct DarkDBPropLoot {
    sint32 gold;
    sint32 gems;
    sint32 art;
    uint32 special;
};

struct DarkDBPropLootInfo {
    sint32 picks;
    char item[6][64];
    sint32 rarity[6];
    sint32 extra[6];
};

/* XXX: Fix me */
struct DarkDBPropLoudRoom {
    float value;
};

struct DarkDBPropMapLoc {
    sint32 location;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBPropMapObjIcon;
#else
#define DarkDBPropMapObjIcon DarkLabel
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropMapObjRotate;
#else
#define DarkDBPropMapObjRotate DarkBoolean
#endif

struct DarkDBPropMapRef {
    sint32 x, y;
    sint32 frame;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropMapText;
#else
#define DarkDBPropMapText DarkString
#endif

struct DarkDBPropMaterial {
    sint32 value;
    char tags[252];
};

struct DarkDBPropMaxHP {
    sint32 hp;
};

struct DarkDBPropMaxSpchPause {
    sint32 pausetime;
};

struct DarkDBPropMelDesImpct {
    sint32 destroychance;
};

struct DarkDBPropMeleeType {
    uint32 type;
};

#pragma pack(push, 2)
struct DarkDBPropMeshAttach {
    uint32 custom;
    uint32 accessory;
    struct {
        sint32 object;
        uint32 joint;
        SCoord angle;
        uint16 zero;
        Coord offset;
    } attachment[4];
};
#pragma pack(pop)

struct DarkDBPropMeshTextures {
    struct {
        char original[16];
        char replacement[16];
    } texture[12];
    char zero[12];
};
#define PROPVERSION_MESHTEXTURES 0x0001018C

#ifdef __cplusplus
typedef DarkString DarkDBPropMetapropType;
#else
#define DarkDBPropMetapropType DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropModeChangeMetaproperty;
#else
#define DarkDBPropModeChangeMetaproperty DarkString
#endif

struct DarkDBPropMiniGames {
    uint32 game;
};

struct DarkDBPropMinSpchPause {
    sint32 pausetime;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBPropModelName;
#else
#define DarkDBPropModelName DarkLabel
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropModeUnchngeMetaproperty;
#else
#define DarkDBPropModeUnchngeMetaproperty DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropModifyText;
#else
#define DarkDBPropModifyText DarkString
#endif
#define DarkDBPropModify1 DarkDBPropModifyText
#define DarkDBPropModify2 DarkDBPropModifyText

struct DarkDBPropMotActorTags {
    char tags[800];
};

struct DarkDBPropMotGaitDescription {
    sint32 offsetleft;
    sint32 offsetright;
    float ascend;
    float descend;
    float timewarp;
    float stretch;
    sint32 anglevelocity;
    sint32 turntolerance;
    float maxvelocity;
    float minvelocity;
    sint32 numturns;
    struct {
        sint32 angle;
        sint32 offset;
    } turn[3];
    float noise;
};

struct DarkDBProp1MotGaitDescription {
    sint32 offsetleft;
    sint32 offsetright;
    float ascend;
    float descend;
    float timewarp;
    float stretch;
    sint32 anglevelocity;
    sint32 turntolerance;
    float maxvelocity;
    float minvelocity;
    sint32 numturns;
    struct {
        sint32 angle;
        sint32 offset;
    } turn[3];
};

struct DarkDBPropMotorController {
    uint32 type;
};

/* XXX: Fix me */
struct DarkDBPropMotPhysLimits {
    sint32 unknown1;
    sint32 unknown2;
};

struct DarkDBPropMotPlyrLimbOffset {
    Coord position;
    Coord angle;
};

/* XXX: Fix me */
struct DarkDBPropMovingTerrain {
    Boolean active;
    sint32 unknown;
};
#define PROPVERSION_MOVINGTERRAIN 0x00010008

struct DarkDBPropMsgTime {
    sint32 time;
};

struct DarkDBPropNameType {
    uint32 type;
};

struct DarkDBPropNetworkCategory {
    uint32 type;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropNoBorrow;
#else
#define DarkDBPropNoBorrow DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropNoDrop;
#else
#define DarkDBPropNoDrop DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropNoFlash;
#else
#define DarkDBPropNoFlash DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropNonPhysCreature;
#else
#define DarkDBPropNonPhysCreature DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropNotPullable;
#else
#define DarkDBPropNotPullable DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropObjIcon;
#else
#define DarkDBPropObjIcon DarkLabel
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropObjBrokenIcon;
#else
#define DarkDBPropObjBrokenIcon DarkLabel
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropObjList;
#else
#define DarkDBPropObjList DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropObjLookS;
#else
#define DarkDBPropObjLookS DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropObjName;
#else
#define DarkDBPropObjName DarkString
#endif
#define PROPVERSION_OBJNAME 0x00020011

#ifdef __cplusplus
typedef DarkString DarkDBPropObjShort;
#else
#define DarkDBPropObjShort DarkString
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropObjShadow;
#else
#define DarkDBPropObjShadow DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkLabel DarkDBPropObjSoundName;
#else
#define DarkDBPropObjSoundName DarkLabel
#endif

struct DarkDBPropObjState {
    uint32 state;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropOTxtRepr;
#else
#define DarkDBPropOTxtRepr DarkString
#endif

/* XXX: Eh? */
struct DarkDBPropParticle {
    sint32 value;
};

#pragma pack(push, 1)
struct DarkDBPropParticleGroup {
    sint32 unknown1[10];
    /*28*/ uint32 type;
    /*2C*/ uint32 motion;
    /*30*/ uint32 animation;
    sint32 unknown2[2];
    /*3C*/ sint32 numparticles;
    sint32 unknown3[6];
    /*58*/ Coord velocity;
    /*64*/ Coord gravity;
    /*70*/ uint8 colors[3]; /* From palette */
    /*73*/ uint8 alpha;
    /*74*/ uint8 simalways;
    /*75*/ uint8 simgroup;
    uint8 zero1;
    /*77*/ uint8 sortparticles;
    uint8 zero2;
    uint8 zero3;
    /*7A*/ uint8 ignorerefs;
    uint8 zero4[5];
    /*80*/ Coord spinspeed;
    /*8C*/ sint32 pulsecycle;
    /*90*/ float pulsesize;
    /*94*/ float radius;
    /*98*/ uint8 startlaunched;
    /*99*/ uint8 spinandpulse;
    /*9A*/ uint8 subpixelalpha;
    /*9B*/ uint8 subpixelskip;
    uint8 zero5[3];
    /*9F*/ uint8 active;
    /*A0*/ sint32 animtimeoffset;
    /*A4*/ float particlesize;
    sint32 unknown4[5];
    /*BC*/ float scalespeed;
    sint32 unknown5[15];
    /*FC*/ uint32 launchtime;
    /*100*/ char bitmap[16];
    sint32 unknown6;
    /*114*/ uint32 fadetime;
    sint32 unknown7[11];
    /*144*/ Boolean matchunrefs;
};

struct DarkDBProp1ParticleGroup {
    sint32 unknown1[10];
    /*28*/ uint32 type;
    /*2C*/ uint32 motion;
    /*30*/ uint32 animation;
    sint32 unknown2[2];
    /*3C*/ sint32 numparticles;
    sint32 unknown3[6];
    /*58*/ Coord velocity;
    /*64*/ Coord gravity;
    /*70*/ uint8 colors[3]; /* From palette */
    /*73*/ uint8 alpha;
    /*74*/ uint8 simalways;
    /*75*/ uint8 simgroup;
    uint8 zero1;
    /*77*/ uint8 sortparticles;
    uint8 zero2;
    uint8 zero3;
    /*7A*/ uint8 ignorerefs;
    uint8 zero4[5];
    /*80*/ Coord spinspeed;
    /*8C*/ sint32 pulsecycle;
    /*90*/ float pulsesize;
    /*94*/ float radius;
    /*98*/ uint8 startlaunched;
    /*99*/ uint8 spinandpulse;
    /*9A*/ uint8 subpixelalpha;
    /*9B*/ uint8 subpixelskip;
    uint8 zero5[3];
    /*9F*/ uint8 active;
    /*A0*/ sint32 animtimeoffset;
    /*A4*/ float particlesize;
    sint32 unknown4[5];
    /*BC*/ float scalespeed;
    sint32 unknown5[15];
    /*FC*/ uint32 launchtime;
    /*100*/ char bitmap[16];
    sint32 unknown6;
    /*114*/ uint32 fadetime;
    sint32 unknown7[11];
};
#pragma pack(pop)
#define PROPVERSION1_PARTICLEGROUP 0x000003ED
#define PROPVERSION2_PARTICLEGROUP 0x000003EE

#ifdef __cplusplus
typedef DarkString DarkDBPropParticleType;
#else
#define DarkDBPropParticleType DarkString
#endif

#pragma pack(push, 2)
struct DarkDBPropPGLaunchInfo {
    uint32 type;
    Coord boxmin;
    Coord boxmax;
    Coord velocitymin;
    Coord velocitymax;
    float radiusmin;
    float radiusmax;
    float timemin;
    float timemax;
    Boolean worldvelocity;
    Boolean worldlocation;
    sint32 unknown[16];
    /*
            uint16	;
            uint32	;	uint32	;	uint32	;
            uint32	;	uint32	;	uint32	;
            uint32	;	uint32	;	uint32	;
            uint32	;	uint32	;
            sint32	;	sint32	;
            sint32	;	sint32	;
            uint16	;
    */
};
#pragma pack(pop)
#define PROPVERSION_PGLAUNCHINFO 0x000003E8

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropPhysAICollides;
#else
#define DarkDBPropPhysAICollides DarkBoolean
#endif

/* XXX: Verify */
struct DarkDBPropPhysAttr {
    float gravity;
    float mass;
    float density;
    float elasticity;
    float friction;
    Coord cog;
    uint32 rotationaxes;
    uint32 restaxes;
    uint32 climbable;
    Boolean edgetrigger;
    float poresize;
};

struct DarkDBProp1PhysAttr {
    float gravity;
    float mass;
    float density;
    float elasticity;
    float friction;
    Coord cog;
    uint32 rotationaxes;
    uint32 restaxes;
    uint32 climbable;
    Boolean edgetrigger;
};
#define PROPVERSION1_PHYSATTRIBUTES 0x000103E8
#define PROPVERSION2_PHYSATTRIBUTES 0x000203E9

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropPhysCanMantle;
#else
#define DarkDBPropPhysCanMantle DarkBoolean
#endif

struct DarkDBPropPhysControls {
    uint32 flags;
    Coord translation;
    Coord velocity;
    Coord rotation;
};
#define PROPVERSION_PHYSCONTROLS 0x000103E8

struct DarkDBPropPhysDims {
    float radius1;
    float radius2;
    Coord offset1;
    Coord offset2;
    Coord size;
    Boolean pointterrain;
    Boolean pointspecial;
};

struct DarkDBProp1PhysDims {
    float radius1;
    float radius2;
    Coord offset1;
    Coord offset2;
    Coord size;
    Boolean pointterrain;
};
#define PROPVERSION1_PHYSDIMS 0x00010030
#define PROPVERSION2_PHYSDIMS 0x000103E9

struct DarkDBPropPhysExplode {
    sint32 magnitude;
    float radius;
};
#define PROPVERSION_PHYSEXPLODE 0x000003E8

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropPhysFaceVelocity;
#else
#define DarkDBPropPhysFaceVelocity DarkBoolean
#endif

struct DarkDBPropPhysInitVelocity {
    float x, y, z;
};

struct DarkDBPropPhysPPlate {
    float weight;
    float travel;
    float speed;
    float pause;
    Boolean blockvision;
    sint32 zero[2];
};
#define PROPVERSION_PHYSPPLATE 0x000103E8

struct DarkDBPropPhysRope {
    float length;
    Boolean deployed;
    float desiredlength;
};
#define PROPVERSION_PHYSROPE 0x000103E9

struct DarkDBPropPhysState {
    Coord location;
    Coord facing;
    Coord velocity;
    Coord rotvelocity;
};
#define PROPVERSION_PHYSSTATE 0x00010030

struct DarkDBPropPhysType {
    uint32 type;
    sint32 submodels;
    Boolean removeonsleep;
    Boolean special;
};
#define PROPVERSION1_PHYSTYPE 0x000103E9
#define PROPVERSION2_PHYSTYPE 0x000103EA

struct DarkDBProp1PhysType {
    uint32 type;
    sint32 submodels;
    Boolean removeonsleep;
};

struct DarkDBPropPickBias {
    float value;
};

#pragma pack(push, 2)
struct DarkDBPropPickCfg {
    struct {
        uint32 lockbits;
        sint16 pins;
        sint16 percent;
        uint32 flags;
    } stage[3];
};
#pragma pack(pop)

struct DarkDBPropPickDistance {
    float distance;
};

struct DarkDBPropPickSrc {
    uint32 bits;
};

#pragma pack(push, 1)
struct DarkDBPropPickState {
    uint8 tumbler;
    uint8 pin;
    uint8 done;
    uint8 randtime;
    sint32 totaltime;
    sint32 stagetime;
    sint32 picker;
};
#pragma pack(pop)

#pragma pack(push, 2)
struct DarkDBPropPlayerGunDesc {
    uint32 flags;
    char hand[16];
    char icon[16];
    Coord modeloffset;
    Coord fireoffset;
    uint16 modelheading;
    uint16 reloadpitch;
    uint16 reloadrate;
    uint32 type;
};
#pragma pack(pop)

#ifdef __cplusplus
typedef DarkString DarkDBPropPlayerNameDesc;
#else
#define DarkDBPropPlayerNameDesc DarkString
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropPlotCritical;
#else
#define DarkDBPropPlotCritical DarkBoolean
#endif

struct DarkDBPropPosition {
    Coord position;
    sint16 cell;
    sint16 zero;
    SCoord facing;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropPreload;
#else
#define DarkDBPropPreload DarkBoolean
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropPrjSound;
#else
#define DarkDBPropPrjSound DarkString
#endif

#pragma pack(push, 2)
struct DarkDBPropProjectile {
    sint32 spray;
    sint16 spread;
};
#pragma pack(pop)
#define PROPVERSION_PROJECTILE 0x00010006

struct DarkDBPropProtocolExplodeRange {
    float distance;
};

struct DarkDBPropProxyStore {
    sint32 value;
};

struct DarkDBPropPsiOverDesc {
    sint32 state;
};
#define DarkDBPropPsiOver2Desc DarkDBPropPsiOverDesc

struct DarkDBPropPsiPower {
    uint32 power;
    uint32 type;
    sint32 initialcost;
    float data[4];
};

struct DarkDBPropPsiPowerDesc {
    uint32 powers;
};
#define DarkDBPropPsiPower2Desc DarkDBPropPsiPowerDesc

struct DarkDBPropPsiRadar {
    uint32 type;
};
#define PROPVERSION_PSIRADAR 0x00010004

struct DarkDBPropPsiShield {
    sint32 basetime;
    sint32 addtime;
    sint32 baseintensity;
};

struct DarkDBPropPsiState {
    uint32 power;
    sint32 level;
    sint32 maxlevel;
};

struct DarkDBPropPsiTags {
    sint32 value;
    char tags[252];
};

/* XXX: Fix me */
struct DarkDBPropPuppet {
    sint32 unknown1;
    sint32 object;
    sint32 zero;
    sint32 unknown2;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropQBName;
#else
#define DarkDBPropQBName DarkString
#endif

struct DarkDBPropQBVal {
    sint32 value;
};

struct DarkDBPropRadAbsorb {
    float amount;
};

struct DarkDBPropRadAmb {
    float level;
};

struct DarkDBPropRadDrain {
    float amount;
};

struct DarkDBPropRadLevel {
    float level;
};

struct DarkDBPropRadRecover {
    float rate;
};

/* XXX: Fix me */
struct DarkDBPropRangedAudioResponse {
    float value1;
    float value2;
    float value3;
};

struct DarkDBPropRecycle {
    sint32 value;
};

struct DarkDBPropRenderAlpha {
    float alpha;
};
#define PROPVERSION_RENDERALPHA 0x00010004

#pragma pack(push, 1)
struct DarkDBPropRenderFlash {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 active;
    sint32 worldduration;
    sint32 screenduration;
    sint32 effectduration;
    float overalltime;
    float range;
    uint32 startframe;
};
#pragma pack(pop)

struct DarkDBPropRenderType {
    uint32 mode;
};

struct DarkDBPropRepContents {
    char name[6][64];
    sint32 cost[6];
};
#define DarkDBPropRepHacked DarkDBPropRepContents;

#ifdef __cplusplus
typedef DarkLabel DarkDBPropRGuarLoot;
#else
#define DarkDBPropRGuarLoot DarkLabel
#endif

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropRngdGruntAlways;
#else
#define DarkDBPropRngdGruntAlways DarkBoolean
#endif

struct DarkDBPropRoomGravity {
    sint32 gravity;
};
#define PROPVERSION_ROOMGRAVITY 0x00010004

#pragma pack(push, 2)
struct DarkDBPropRotDoor {
    sint32 zero;
    float closedangle;
    float openangle;
    float speed;
    uint32 axis;
    uint32 state;
    Boolean hardlimits;
    float blocksound;
    Boolean blockvision;
    float pushmass;
    Coord closedposition;
    Coord openposition;
    Coord startposition;
    SCoord startfacing; /* must be closed */
    float unknown;
    sint32 room[2];
    Boolean clockwise;
    /* haltclose and haltopen must be inside the arc of rotation */
    /* i.e. if clockwise, haltclose = closedangle + 2 and haltopen = openangle -
     * 2 */
    SCoord haltclose;
    SCoord haltopen;
};
#pragma pack(pop)
#define PROPVERSION_ROTDOOR 0x000103E9

/* The research type is a bit-vector that is translated into
 * an index in strings/research.str. Only the lowest set bit
 * is recognized. Dromed only lets you set 24 bits, but all 32
 * will be recognized by the game.
 */
struct DarkDBPropRsrchRep {
    uint32 objecttype;
};

struct DarkDBPropRsrchTime {
    sint32 time;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropRsrchTxt;
#else
#define DarkDBPropRsrchTxt DarkString
#endif

struct DarkDBPropSalePrice {
    sint32 price;
};
#define PROPVERSION_SALEPRICE 0x00010004

struct DarkDBPropScale {
    float x, y, z;
};

#ifdef __cplusplus
typedef DarkLabel DarkDBPropSchActionSound;
#else
#define DarkDBPropSchActionSound DarkLabel
#endif

struct DarkDBPropSchAttFactor {
    float attenuation;
};

struct DarkDBPropSchLastSample {
    int value;
};

#pragma pack(push, 1)
struct DarkDBPropSchLoopParams {
    uint8 flags;
    uint8 maxsamples;
    sint16 loopcount;
    sint16 mininterval;
    sint16 maxinterval;
};
#pragma pack(pop)

#ifdef __cplusplus
typedef DarkLabel DarkDBPropSchMsg;
#else
#define DarkDBPropSchMsg DarkLabel
#endif

struct DarkDBPropSchPlayParams {
    uint16 flags;
    uint16 audioclass;
    sint32 volume;
    sint32 pan;
    uint32 delay;
    sint32 fade;
};
#define SCHEMAPARAMS_RETRIGGER 0x0001
#define SCHEMAPARAMS_PANPOSITION 0x0002
#define SCHEMAPARAMS_PANRANGE 0x0004
#define SCHEMAPARAMS_NOREPEAT 0x0008
#define SCHEMAPARAMS_NOCACHE 0x0010
#define SCHEMAPARAMS_STREAM 0x0020
#define SCHEMAPARAMS_PLAYONCE 0x0040
#define SCHEMAPARAMS_NOCOMBAT 0x0080

#define SCHEMAPARAMS_NOISE 1
#define SCHEMAPARAMS_SPEECH 2
#define SCHEMAPARAMS_AMBIENT 3
#define SCHEMAPARAMS_MUSIC 4
#define SCHEMAPARAMS_METAUI 5
#define SCHEMAPARAMS_PLAYERFEET 6
#define SCHEMAPARAMS_OTHERFEET 7
#define SCHEMAPARAMS_COLLISIONS 8
#define SCHEMAPARAMS_WEAPONS 9
#define SCHEMAPARAMS_MONSTERS 10

struct DarkDBPropSchPriority {
    sint32 priority;
};

struct DarkDBPropScripts {
    char script[4][32];
    Boolean dontinherit;
};
#define PROPVERSION_SCRIPTS 0x000A0134

struct DarkDBPropScriptTiming {
    sint32 timing;
};

struct DarkDBPropSelfIllum {
    float brightness;
};
#define PROPVERSION_SELFILLUM 0x00010004

struct DarkDBPropSelfLit {
    sint32 brightness;
};

struct DarkDBPropService {
    uint32 service;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropSettingText;
#else
#define DarkDBPropSettingText DarkString
#endif
#define DarkDBPropSett1 DarkDBPropSettingText
#define DarkDBPropSett2 DarkDBPropSettingText

#ifdef __cplusplus
typedef DarkString DarkDBPropSettingHeader;
#else
#define DarkDBPropSettingHeader DarkString
#endif
#define DarkDBPropSHead1 DarkDBPropSettingHeader
#define DarkDBPropSHead2 DarkDBPropSettingHeader

struct DarkDBPropShadow {
    sint32 intensity;
};

struct DarkDBPropShakeAmt {
    sint32 strength;
};

struct DarkDBPropShkAIRanged {
    sint32 mindistance;
    sint32 maxdistance;
    float firedelay;
    uint32 firewhenmoving;
};

struct DarkDBPropShockWeaponType {
    uint32 type;
};

struct DarkDBPropShodanExplodeRange {
    float distance;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropShowHP;
#else
#define DarkDBPropShowHP DarkBoolean
#endif

/* XXX: eh? */
struct DarkDBPropShove {
    float x, y, z;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropSignalType;
#else
#define DarkDBPropSignalType DarkString
#endif

struct DarkDBPropSlayResult {
    uint32 mode;
};

struct DarkDBPropSoftLevel {
    sint32 softwarelevel;
};

struct DarkDBPropSoftType {
    uint32 softwaretype;
};

#pragma pack(push, 2)
struct DarkDBPropSpark {
    uint32 flags;
    uint16 color;
    uint16 anglejitter;
    uint16 angle[3];
    uint16 zero;
    float size[3];
    float sizejitter;
    float light;
    float lightjitter;
};
#pragma pack(pop)

struct DarkDBPropSpawn {
    char type[4][64];
    sint32 rarity[4];
    uint32 flags;
    sint32 count;
};

struct DarkDBPropSpchNextPlay {
    sint32 value;
};

struct DarkDBPropSpchPause {
    sint32 duration;
};
#define DarkDBPropMinSpchPause DarkDBPropSpchPause
#define DarkDBPropMaxSpchPause DarkDBPropSpchPause

#ifdef __cplusplus
typedef DarkLabel DarkDBPropSpchVoice;
#else
#define DarkDBPropSpchVoice DarkLabel
#endif

struct DarkDBPropSpeech {
    uint32 flags;
    sint32 time;
    sint32 schema;
    sint32 concept;
    sint32 unknown;
    uint32 handle;
};

struct DarkDBPropSpotAmbient {
    float inner;
    float outer;
    float ambient;
};
#define PROPVERSION_SPOTAMBIENT 0x0001000C

struct DarkDBPropSpotlight {
    float inner;
    float outer;
    float distance;
};
#define PROPVERSION_SPOTLIGHT 0x0001000C

struct DarkDBPropSrcRand {
    float factor;
};

struct DarkDBPropStackCount {
    sint32 count;
};

struct DarkDBPropStackInc {
    sint32 increment;
};

struct DarkDBPropStartLoc {
    sint32 location;
};

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropStimKO;
#else
#define DarkDBPropStimKO DarkBoolean
#endif

#pragma pack(push, 2)
struct DarkDBPropStTweqBlink {
    uint16 anim;
    uint16 misc;
    uint16 time;
    uint16 frame;
};

struct DarkDBPropStTweqDelete {
    uint16 anim;
    uint16 misc;
    uint16 time;
    uint16 frame;
};

struct DarkDBPropStTweqEmit {
    uint16 anim;
    uint16 misc;
    uint16 time;
    uint16 frame;
};
#define DarkDBPropStTweq2Emit DarkDBPropStTweqEmit
#define DarkDBPropStTweq3Emit DarkDBPropStTweqEmit
#define DarkDBPropStTweq4Emit DarkDBPropStTweqEmit
#define DarkDBPropStTweq5Emit DarkDBPropStTweqEmit

struct DarkDBPropStTweqJoint {
    uint16 anim;
    uint16 misc;
    uint32 joint[6];
};

struct DarkDBPropStTweqLock {
    uint16 anim;
    uint16 misc;
    float angle;
    uint32 stage;
};

struct DarkDBPropStTweqModel {
    uint16 anim;
    uint16 misc;
    uint16 time;
    uint16 frame;
};

struct DarkDBPropStTweqRotate {
    uint16 anim;
    uint16 misc;
    uint32 x, y, z;
};

struct DarkDBPropStTweqScale {
    uint16 anim;
    uint16 misc;
    uint32 x, y, z;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DarkDBPropSuspObject {
    uint8 issuspicious;
    char type[32];
    float minlight;
};
#pragma pack(pop)

struct DarkDBPropSwingExposure {
    sint32 value;
};

struct DarkDBPropSwordAction {
    uint32 mode;
};

#ifdef __cplusplus
typedef DarkString DarkDBPropSymName;
#else
#define DarkDBPropSymName DarkString
#endif

struct DarkDBPropTargetType {
    uint32 type;
};
#define PROPVERSION_TARGETTYPE 0x00010004

struct DarkDBPropTechDiff {
    sint32 successchance;
    sint32 criticalchance;
    float cost;
};
#define DarkDBPropHackDiff DarkDBPropTechDiff
#define DarkDBPropRepairDiff DarkDBPropTechDiff
#define DarkDBPropModifyDiff DarkDBPropTechDiff
#define DarkDBPropModify2Diff DarkDBPropTechDiff

#ifdef __cplusplus
typedef DarkString DarkDBPropTerrReplace;
#else
#define DarkDBPropTerrReplace DarkString
#endif
#define DarkDBPropTerrRepOn DarkDBPropTerrReplace
#define DarkDBPropTerrRepOff DarkDBPropTerrReplace
#define DarkDBPropTerrRepDestroy DarkDBPropTerrReplace

struct DarkDBPropTextureRadius {
    float distance;
};

struct DarkDBPropTimeWarp {
    float factor;
};

struct DarkDBPropToolReach {
    float distance;
};

struct DarkDBPropToxin {
    float amount;
};

struct DarkDBPropTraitsDesc {
    sint32 trait[4];
};

#pragma pack(push, 2)
struct DarkDBPropTransDoor {
    sint32 zero;
    float closed;
    float open;
    float speed;
    uint32 axis;
    uint32 state;
    Boolean hardlimits;
    float blocksound;
    Boolean blockvision;
    float pushmass;
    Coord closedposition;
    Coord openposition;
    Coord startposition;
    SCoord startfacing;
    float unknown;
    sint32 room[2];
};
#pragma pack(pop)
#define PROPVERSION_TRANSDOOR 0x000103E8

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropTransient;
#else
#define DarkDBPropTransient DarkBoolean
#endif

struct DarkDBPropTransRate {
    sint32 translucance;
};

struct DarkDBPropTrapFlags {
    uint32 flags;
};
#define TRAPFLAG_ONCE 0x1
#define TRAPFLAG_INVERT 0x2
#define TRAPFLAG_NOON 0x4
#define TRAPFLAG_NOOFF 0x8

#ifdef __cplusplus
typedef DarkString DarkDBPropTrapQVar;
#else
#define DarkDBPropTrapQVar DarkString
#endif

struct DarkDBPropTripFlags {
    uint32 flags;
};
#define TRIPFLAG_ENTER 0x0001
#define TRIPFLAG_EXIT 0x0002
#define TRIPFLAG_MONO 0x0004
#define TRIPFLAG_ONCE 0x0008
#define TRIPFLAG_INVERT 0x0010
#define TRIPFLAG_PLAYER 0x0020
#define TRIPFLAG_ALARM 0x0040
#define TRIPFLAG_SHOVE 0x0080
#define TRIPFLAG_ZAPINSIDE 0x0100
#define TRIPFLAG_EASTEREGG 0x0200

#ifdef __cplusplus
typedef DarkString DarkDBPropUseCursor;
#else
#define DarkDBPropUseCursor DarkString
#endif

#ifdef __cplusplus
typedef DarkString DarkDBPropUseMsg;
#else
#define DarkDBPropUseMsg DarkString
#endif

struct DarkDBPropUseType {
    uint32 type;
};

struct DarkDBPropVoiceIndex {
    sint32 voice;
};

#pragma pack(push, 1)
struct DarkDBPropWaterColor {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 zero;
    float alpha;
};
#pragma pack(pop)

struct DarkDBPropWeaponDamage {
    sint32 damage;
};

struct DarkDBPropWeaponType {
    sint32 type;
};

struct DarkDBPropWeather {
    sint32 unknown1;
    Boolean fog;
    Boolean precipitation;
    sint32 unknown2;
};
#define PROPVERSION_WEATHER 0x00010010

#ifdef __cplusplus
typedef DarkString DarkDBPropWorldCursor;
#else
#define DarkDBPropWorldCursor DarkString
#endif

struct DarkDBPropWpnExposure {
    sint32 value;
};
#define PROPVERSION_WPNEXPOSURE 0x00010004

#ifdef __cplusplus
typedef DarkBoolean DarkDBPropWpnTerrCollides;
#else
#define DarkDBPropWpnTerrCollides DarkBoolean
#endif

struct DarkDBPropZBias {
    sint32 bias;
};

#ifdef __cplusplus
} // namespace Dark
#endif

#endif
