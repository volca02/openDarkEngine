#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define uint8 uint8_t
#define sint8 int8_t
#define uint16 uint16_t
#define sint16 int16_t
#define sint32 uint32_t
#define uint32 int32_t

#define PI 3.14159265358979323846

#pragma pack(push,1)
typedef struct { // SIZE: 8
	uint32	phys_version;
} phys_hdr;

typedef struct { // SIZE: 12
	float	x;
	float   y;
	float	z;
} t_coord;

// Vector with length/norm (Controls-Velocities use this)
typedef struct { // SIZE: 16
	float	x;
	float   y;
	float	z;
	float	norm;
} t_coord_norm;

// Definition of the facing angle...
typedef struct { // SIZE: 8
	unsigned short x;
	unsigned short y;
	unsigned short z;
} t_facing; // This will be SCord after all, as telliamed writes....

// Once I'll understand what the sub-objects are, I'll work this out :
/*
Remarks:
The facing1 and facing4 seem to be the same value (maybe those differ in the time of collision)
*/
typedef struct { // SIZE: 18 longs (72 bytes)
	t_coord pos1;     //  

	sint32  unk1;     //   TODO: Often -1
	t_facing facing1; //   Sorta Angle of this sub-object element.
	t_facing facing2; //   Sorta Angle of this sub-object element.
	t_facing facing3; //   Angle again - different.
	
	uint32 a2; // Address 
	
	t_facing facing4; //   Angle again - normaly = facing1
	
	
	
	// float   a2, b2;   //   Unknown (Angle releted?)

	t_coord pos2;     //   Seems to be copy of pos1, but matbe not 100% of the time

	uint32 a3, b3, c3, d3;     // Unknown. - Pointers
	
} t_subObject;


typedef struct {
	uint32	contact_type; // 1-5?
	uint32	object_num;
	uint32	unknown;
} t_PhysContactHeader;
#pragma pack(pop)


/* PHYS. OBJECT FLAGS */
#define PHYS_OBJ_ROPE      0x0001000
#define PHYS_OBJ_MTERRAIN  0x0100000

const char hchr[]="0123456789ABCDEF";
////////////////// HELPERS //////////////////
void printPhysObjFlags(uint32 flags) {
	printf("\tFlags         : ");
	
	if (flags & PHYS_OBJ_ROPE)
		printf("ROPE ");
	
	if (flags & PHYS_OBJ_MTERRAIN)
		printf("MOVING-TERRAIN ");
	
	printf("\n");
}

void p_coord(t_coord &c) {
	printf("		[ %+10.2f %+10.2f %+10.2f ]\n",c.x,c.y,c.z);
}


void printFacing(t_facing &facing) {
	//printf("X: %8.6g Y: %8.6g Z: %8.6g\n", facing.x * PI / 1024, facing.y * PI / 1024, facing.z * PI / 1024); // I dunno exactly. Well
	printf("X: %6hu Y: %6hu Z: %6hu\n", facing.x, facing.y, facing.z); // I dunno exactly. Well
}


void readFacing(FILE *f) {
	t_facing fc;
	
	fread(&fc, 1, sizeof(t_facing), f);
	
	printFacing(fc);
}

void readSubObject(FILE* f) {
	t_subObject s;
	
	fread(&s, 1, sizeof(t_subObject), f);
	
	printf("\t\tPOS1     : "); p_coord(s.pos1);
	
	printf("\t\tUNK1     : %X\n", s.unk1);
	printf("\t\tFAC 1    : "); printFacing(s.facing1);
	printf("\t\tFAC 2    : "); printFacing(s.facing2);
	printf("\t\tFAC 3    : "); printFacing(s.facing3);
	
	printf("\t\tsa       : %X\n", s.a2);
	
	printf("\t\tFAC 4    : "); printFacing(s.facing4);
	
	printf("\t\tPOS2     : "); p_coord(s.pos2);
	
	printf("\t\tPOINTERS      : %X %X %X %X\n", s.a3, s.b3, s.c3, s.d3);
	
}

void hexdump(unsigned char *ptr,int size, FILE *target=stdout) {
	int i;
	
	for (i=0; i<size; i++,ptr++) {
		if (i % 4 == 0)
		    fprintf(target," ");

		uint16 a = static_cast<uint16>(ptr[0]);
		uint16 b = static_cast<uint16>(ptr[0]);
		a >>= 4;
		b &= 0xF;
		fprintf(target,"%c%c ",hchr[a],hchr[b]);
	}
	fprintf(target,"\n");
}

void hexread(FILE *f,unsigned int size,char *prefix,int llen=16) {
	unsigned int sz = 0;
	if (llen>256) {
		fprintf(stderr,"ERROR: hexread: can't handle more than 256 bytes/line ! "); 
		return;
	}
	
	// file size...
	long apos = ftell(f);
	fseek (f, 0, SEEK_END);
	long lSize = ftell(f);
	fseek (f, apos, SEEK_SET);
	
	while ((sz<size) && (!feof(f))) {
			int chunk = size-sz;
			if (chunk>llen) chunk = llen;

			long remains = lSize - ftell(f); 
			// fprintf(stderr,"%s - Pos %8lX - Remains %ld/%ld bytes\n", prefix, ftell(f), remains, lSize);
			
			if (chunk > remains)
				chunk = remains;
			
			if (chunk == 0)
				break;
			
			uint8 data[256];
			fread(&data,chunk,1,f);
			printf("%s",prefix);
			hexdump(data,chunk);
			sz+=chunk;
		}
}

/// STRUCT PRINTING

// Physics header
void p_HDR(phys_hdr &hdr) {
	printf("Phys Header \n");
	printf("\tVersion      : %d\n", hdr.phys_version);
}

// Bounding volume type printout. There is a difference for 0,2,4 - sphere hat for example
bool p_BVolType(sint32 bv) {
	bool err = false;
	// Telliamed -> 0:Sphere, 1:BSP, 2:Point, 3:OBB, 4:SphereHat, -128:Invalid.
	printf("\tBounding [%1d]   : ", bv);
	switch (bv) {
		case 0:
			printf("Sphere");
			break;
		case 2:
			printf("Point (Sphere)");
			break;
		case 4:
			printf("Sphere Hat");
			break;
		case 1:
			printf("BSP Tree - Object defined boundry");
			break;
		case 3:
			printf("BOX");
			break;
		case -128:
			printf("Invalid");
			break;
		default:
			printf("Unknown boundry!");
			err = true;
			break;
	}
	
	printf("\n");
	return err;
}

void fpos(FILE *f) {
	printf("------------- FPOS %lXh -----------\n",ftell(f));
}

void readMatrixLong(FILE *f, int width, int height, char *prefix) {
	for (int y = 0; y < height; y++) {
		printf("%s[%4d] ", prefix, y);
		for (int x = 0; x < width; x++) {
			sint32 element;
			fread(&element,1,4,f);
			printf("%10d ", element);
		}
		printf("\n");
	}
}

void readMatrixFloat(FILE *f, int width, int height, char *prefix) {
	for (int y = 0; y < height; y++) {
		printf("%s[%4d] ", prefix, y);
		for (int x = 0; x < width; x++) {
			float element;
			fread(&element,1,4,f);
			printf("%8.2g ", element);
		}
		printf("\n");
	}
}

void readVector3(FILE *f) {
	t_coord vect;
	fread(&vect,1,sizeof(t_coord),f);
	
	printf(" X: %8.2g Y: %8.2g Z: %8.2g\n", vect.x, vect.y, vect.z);
}


void readVector3N(FILE *f) {
	t_coord_norm vect;
	fread(&vect,1,sizeof(t_coord_norm),f);
	
	printf(" X: %8.2g Y: %8.2g Z: %8.2g NORM: %8.2g\n", vect.x, vect.y, vect.z, vect.norm);
}

void readStruct(char *format, FILE *f) {
	unsigned int x;
	
	for (x = 0; x < strlen(format); x++) {
		switch (format[x]) {
			case 'F':
			case 'f':
				float fl;
				fread(&fl,1,4,f);
				printf("%8.2gf ",fl);
				break;
			case 'L':
			case 'l':
				sint32 y;
				fread(&y,1,4,f);
				printf("%8dl ",y);
				break;
			case 'U':
			case 'u':
				uint32 z;
				fread(&z,1,4,f);
				printf("%8uu ",z);
				break;
			case 'X':
				uint32 d;
				fread(&d,1,4,f);
				printf("%8XX ",d);
				break;
			case 'x':
				uint16 e;
				fread(&e,1,2,f);
				printf("%8hXx ",e);
				break;
			default:
				printf("?(%c)", format[x]);
		}
	}
}


// This should read the bounding volume definitions
void readBoundingDefinition(FILE *f, uint32 btype, uint32 version, uint32 submodel_count) {
	printf("\tBounding Volume definition   :\n");
	switch (btype) {
		case 0:
		case 2:
		case 4:
			// The radiuses are repeated for all the sub-objects
			for (int n = 0; n < submodel_count; n++) {
				float radius;
				fread(&radius,4,1,f);
				printf("\tSphere %d - Radius %8.2g\n", n, radius);
			}
			break;
		case 1:
			printf("BSP Tree - Object defined boundry (no data)\n");
			break;
		case 3:
			printf("\tBOX : ");
			printf("\tBOUNDING UNKNOWN  : "); readStruct("FFF", f);printf("\n"); // Translation of the obb?	
			printf("\tBox dimensions: "); readVector3(f);
			printf("\tBOUNDING UNKNOWN  : "); readStruct("FF", f);printf("\n"); // Translation of the obb?
			// THIEF 2 has more data here
			if (version >= 32) {
				 printf("\tT2 specif. : "); readStruct("X", f);printf("\n");
			}
			break;
		default:
			printf("Unknown boundry!\n");
			break;
	}
	
}

void printRotationAxes(uint32 ax) {
	printf("\tRotational Axes : ");
	
	if (ax & 4)
		printf("Z ");
	
	if (ax & 2)
		printf("Y ");
	
	if (ax & 1)
		printf("X");
	
	printf("\n");
}

// Prints out the Rest axes. I didn't test this one but it seems to be reasonable that the bit order is the same as for Rot. Axes (Dunno about the +- things)
void printRestAxes(uint32 rest) {
	printf("\tRest Axes  : ");
	
	if (rest & 32)
		printf("+Z ");
	
	if (rest & 16)
		printf("+Y ");
	
	if (rest & 8)
		printf("+X ");
	
	if (rest & 4)
		printf("-Z ");
	
	if (rest & 2)
		printf("-Y ");
	
	if (rest & 1)
		printf("-X");
	
	printf("\n");
}

/// Read one object from the Phys syst
bool readObjectPhys(FILE *f, int pos, int version) {
	printf("  Object %d : \n", pos);
	
	// First uint32 is bounding type
	sint32 bvolume;
	fread(&bvolume,1,4,f);
	if (p_BVolType(bvolume))
		return false;
	
	// 
	uint32 object_id, num_subobjs, phys_flags;
	fread(&object_id,1,4,f);
	fread(&num_subobjs,1,4,f);
	fread(&phys_flags,1,4,f);
	printf("\tObjectID      : %d\n",object_id);
	printf("\tSub-Objects   : %d\n",num_subobjs);
	printf("\tFlags         : %08X\n",phys_flags);
	
	printPhysObjFlags(phys_flags);
	// Will have to investigate what the flags mean. It seems there are flags for doors and translational objects, rotational objects, etc
	
	float gravity;
	fread(&gravity,1,4,f);
	printf("\tGravity       : %f\n", gravity);
	
	if (num_subobjs > 200) {
		fprintf(stderr, "Sub-object count too big to be true - %d - exiting\n", num_subobjs);
		return false;
	}
	
	uint32* subobj_counts = new uint32[num_subobjs];
	fread(subobj_counts,4,num_subobjs,f);
	
	int total_subobjdata = 0;
	
	printf("\tSubobj Counts : ");
	for (int n = 0; n < num_subobjs; n++) {
		printf("%d", subobj_counts[n]);
		total_subobjdata += subobj_counts[n];
		if (n < num_subobjs-1)
			printf(", ");
	}
	printf("\n");
	
	// After the subobject integers, there is a friction float
	
	float friction;
	fread(&friction,1,4,f);
	printf("\tBase Friction : %f\n", friction);
	
	// This looks like some other physical value, dunno which
	float unknown;
	fread(&unknown,1,4,f);
	printf("\tUnknown       : %f\n", unknown);

	fpos(f);
	
	
	// This looks rope releted
	printf("\t Float List for sub-objects (%d) : \n", num_subobjs);
	for (int n = 0; n < num_subobjs; n++) {
		printf("\t\t %d : ", n);
		//
		float x;
		
		fread(&x,1,4,f);
		printf("%8.2g ", x);
		
		fread(&x,1,4,f);
		printf("%8.2g ", x);
		
		// 
		printf("\n");
	}
	
	fpos(f);
	printf("\tUnknown    : "); readStruct("LLLL", f);printf("\n");

	/* 
	Rotation axises flags: X - 1, Y - 2, Z - 4 ? (Sphere - all = 7, box = 4 - X only?)
	Rests: 63 for cube.
	*/
	uint32 rot_flags, rest_flags; 
	fread(&rot_flags,1,4,f);
	fread(&rest_flags,1,4,f);
	
	printRotationAxes(rot_flags);
	printRestAxes(rest_flags);
	

	printf("\tUnknown    : "); readStruct("LLL", f);printf("\n");
	
	// THIEF 2 has one more long here
	if (version >= 32) {
		printf("\tT2 specif. : "); readStruct("L", f);printf("\n");
	}
	
	printf("\n");
	fpos(f);
	
	
	printf("\tUnknown  : "); readStruct("FFFFL", f);printf("\n");
	fpos(f);
	
	
	/*
	The last sub-object is set to the center of the Collision Volume.
	*/
	for (int n = 0; n < num_subobjs + 1; n++) {
		// printf("\t SUBOBJ[%4d] : ",n); readStruct("FFFLFFFFXFFFFFLFXX", f);printf("\n");
		// printf("\t SUBOBJ[%4d] : ",n); readStruct("FFFXXXFFXXXFFFXXXX", f);printf("\n");
		printf("\t SUBOBJ[%4d] :\n",n); readSubObject(f);
	}
	
	
	fpos(f);
	printf("\tCOG Offset : "); readVector3(f);
	
	// Sub Object direction Vectors (translations from the center)
	fpos(f);
	for (int n = 0; n < num_subobjs; n++) {
		printf("\tSubObj Translation vec. [%6d] : ", n); readVector3(f);
	}
	fpos(f);
	
	// Count of the velocity descriptions... I have to look for the right place to cut the following structures. Velocity for example is defined twice...
	uint32 vel_counts;
	fread(&vel_counts,1,4,f);
	printf("\tVel. counts : %d\n", vel_counts); 
	fpos(f);
	
	if (vel_counts > 2) {
		printf("Too big vel_count!\n");
		return false;
	}
	
	printf("\tPointers        : "); readStruct("XXXXXXX", f); printf("\n");

	fpos(f);
	
	// Those are the global? Speed values
	// Translation speed here
	printf("\tVelocity : "); readVector3(f);
	
	// Axial vel?
	printf("\tAxial v.?: "); readVector3(f);
	printf("\tPointers?: "); readStruct("XXXX", f);printf("\n");
	//printf("\tUnknown  : "); readFacing(f);
	//printf("\tUnknown  : "); readFacing(f);
	// Rotational speed ?
	printf("\tRot vel. : "); readVector3(f);

	fpos(f);
	printf("\t???      : "); readStruct("LLF",f);printf("\n");
	fpos(f);
	
	// These fit well
	float mass, density, elasticity;
	fread(&mass, 1,4,f);
	fread(&density, 1,4,f);
	fread(&elasticity, 1,4,f);
	
	printf("\tMass     : %8.4g\n", mass);
	printf("\tDensity  : %8.4g\n", density);
	printf("\tElast.   : %8.4g\n", elasticity);
	
	
	
	printf("\tPtrs?    : "); readStruct("XXX", f);printf("\n");
	
	uint32 count_submodels; // THE NUMBER of sub-models again. Realy.
	fread(&count_submodels,1,4,f);
	printf("\tSubmodel cnt  : %d\n", count_submodels);
	
	fpos(f);
	
	/*
	Following structures seem like per sub-model velocities. This makes sense, As this is probably used for objects with movable parts.
	The sub-object - sub-model mapping is probably made by order of appearence
	*/
	
	// Dunno if this condition is right. count_submodels > 1 didn't work well
	if (vel_counts >= 2) {
		// should be while sub model number >0 maybe
		for (int a = 0; a < count_submodels; a++) {
			fpos(f);
			
			// Something like a signature (54 18 BE 01)
			printf("\tPointer   : "); readStruct("X", f);printf("\n");
			
			// Subobject index
			uint32 subobj_idx;
			fread(&subobj_idx,1,4,f);
			printf("\tSubobject index : %d\n", subobj_idx);
			
			// Now follow 108 bytes (27 longs)
			// The first is very often set to 03
			// also, there is a velocity vector inside... printf("\tVelocity : "); readVector3(f); 
			
			printf("\tUnknown  : "); readStruct("XX", f);printf("\n");
			
			// 3,3,3,4,3,3,4,2
			printf("\tVel1 ?   : "); readVector3(f); // Probably not floats but hexadecimal (pointers?)
			printf("\tVel2     : "); readVector3(f);
			printf("\tVel3     : "); readVector3(f);
			printf("\tVel4 ?   : "); readVector3N(f); // Probably not floats but hexadecimal (pointers?)
			printf("\tVel5     : "); readVector3(f);
			printf("\tVel6     : "); readVector3(f);
			printf("\tVel7     : "); readVector3(f);
			printf("\tVel8 ?   : "); readVector3(f); // Probably not floats but hexadecimal (pointers?)
			
			// not a matrix, but 25 bytes length it has. there are 3 floats on [3][4],[4][0],[4][1]  (were one usualy)
			//readMatrixLong(f,5,5,"\tUnknown");
		}
		
		// If we're here, a number indicating some count is present
		printf("\tCount What? : "); readStruct("L", f);printf("\n");
	}
	
	fpos(f);
	
	// Flags for the Controls Follow (BOX offset: 0x374, Sphere offset: 0x20c, Two-sphered object: 0x2e0, SphereHat 0x2e0, 3 sphered rope 0x3b4, 4sph. rope 0x488)
	// Control flags have: -1 for global ones, and, if there is more than 1 sub-model, they repeat for all of them
	/*
	4 sub-model offsets (the index number offset):
	-1 : 
	*/
	int control_counts = 1;
	
	if (vel_counts >= 2)
		control_counts = count_submodels + 1;
	
	for (int a = 0; a < control_counts; a++) {
		fpos(f);
		// Maybe not, but seems like a signature/pointer
		printf("\tPointer    : "); readStruct("X", f);printf("\n");
		
		// Subobject index
		sint32 minusone;
		fread(&minusone,1,4,f);
		printf("\tSubmodel index (-1 = global controls) : %d\n", minusone);
		
		/// Control flags. min
		// Should be 0x2e0 for 2Subobj sphere (is 0x26c - 116 bytes less than should (29 Long/float values) )
		/*
		8 - rotation
		*/
		uint32 control_flags;
		fread(&control_flags,1,4,f);
		printf("\tControl Flags : %08X\n", control_flags);
		
		// Position?
		printf("\tUnknown       : "); readVector3N(f);
		
		// Now the control vectors (t_coord_norm * 4?)
		printf("\tAxis Velocity : "); readVector3N(f);
		printf("\tVelocity      : "); readVector3N(f);
		printf("\tRot. Velocity : "); readVector3N(f);

		// I think this could be a rotation vector... It was nonzero for moving object
		// printf("\tUnk. Vector 1  : "); readVector3(f);
		if ( a == 0 && (control_counts > 1)) {
			printf("\tSob-model control count : "); readStruct("L", f);printf("\n");
		}
	}
	
	// We should be on the right offset for the bounding volume definition readout
	
	// Hey! There are probably other data here if the sub_model count is > 1
	
	
	readBoundingDefinition(f, bvolume, version, num_subobjs);


	// Actually the BOX type phys has 8 bytes more per chunk with one box inside

	/*
	Sphere has 308 Bytes less than BOX
	
	1st sphere has radius at 0x2A0 from the file start
	
	Box has 0x3c4 offset for bounding dimensions...
	
	3f0 - 3e4 = 14 Bytes more than I should
	
	- The next object begins:
	2B0 for sphere
	3d8 for obb
	*/
	
	fpos(f);
	
	// release sub-obj data
	delete subobj_counts;
	
	if (feof(f)) {
		return false;
	}
	return true;
}

void printPhysContactHeader(t_PhysContactHeader& hdr) {
	printf("\tPhys Contact Header:\n");
	printf("\t\tContact type  : %d\n", hdr.contact_type);
	printf("\t\tObject Number : %d\n", hdr.object_num);
	printf("\t\tUnknown       : %d\n", hdr.unknown);
}

void readContacts(FILE *f) {
	// Well, this should be parametrised somewhere. Not a while feof no no no.
	while (!feof(f)) {
		fpos(f);
		t_PhysContactHeader pchdr;
		fread(&pchdr,1,sizeof(t_PhysContactHeader),f);
	
		printPhysContactHeader(pchdr);
		
		fpos(f);
		
		switch (pchdr.contact_type) {
			case 1:
				printf("\tUnk     : "); readStruct("X", f); printf("\n"); 
	
				// A vector3
				printf("\tUnk vec : "); readVector3(f);
			
				// Some number
				printf("\tUnk     : "); readStruct("X", f); printf("\n"); 
		
				// Contact list. Normal, and distance. (I suppose these are from the object COG or center)
				uint32 count;
				fread(&count, sizeof(uint32), 1, f);
	
				printf("\tCount   : %d\n", count);
						
				readMatrixFloat(f,4,count,"\tContact");
			
				fread(&count, sizeof(uint32), 1, f);
	
				// Objects involved in the contact?
				printf("\tObject links Count : %d\n", count);
			
				readMatrixLong(f,1,count,"\tUnk");
				
				
				fread(&count, sizeof(uint32), 1, f);
	
				printf("\tCount   : %d\n", count);
			
				readMatrixLong(f,1,count,"\tUnk");
				
				// sorta pointer here
				
				break;
			default:
				printf("Unknown contact type - %d. ending readout...\n", pchdr.contact_type);
				return;
		}
	}
	
}

//////////////////// MAIN ////////////////////
int main(int argc, char *argv[]) {
	FILE	*f;
	
	phys_hdr hdr;
	
	fprintf(stderr,"===== read PHYS_SYSTEM chunk decomposer =====\n");
	
	if (argc<2) {
		fprintf(stderr,"Please specify a valid filename as argument.\n");
		return 1;
	}
	
	f = fopen(argv[1],"rb");
	
	if (f == NULL) {
		fprintf(stderr,"File %s could not be opened!\n",argv[1]);
		return(1);
	}
	
	fprintf(stderr,"Processing '%s'\n", argv[1]);
	
	// we have the file ready
	fread(&hdr,sizeof(hdr),1,f);
	p_HDR(hdr);
	
	fprintf(stderr, " * Phys. system version : '%d'\n", hdr.phys_version);
	
	for (int group=0; group < 3; group++) {
		uint32 obj_count;
		fread(&obj_count,sizeof(obj_count),1,f);
		printf("Group %d object count : %d\n", group, obj_count);
		
		for (int n = 0; n < obj_count; n++) {
			if (!readObjectPhys(f, n, hdr.phys_version))  {
				fprintf(stderr,"Error encountered, ending readout\n");
				break;
			}
		}
	}
	
	// Contacts: "5 3 0" for empty?
	fpos(f);
	
	// something here I think. Can be the 4. list of objects, or something else.... dunno.
	uint32 len;
	fread(&len,sizeof(len),1,f);
	printf("Extra data count : %d\n", len);
	fpos(f);
	
	readContacts(f);
	
	fpos(f);
	
	if (feof(f))
		printf("EOF\n");
	
	fclose(f);
	
}

/*
There are 3 or 4 sections of physical object definitions (not sure yet). 
The first contains the active objects
The second contains sleeping objects
I dunno about the last 1-2 (Was/Were empty in the savegames of T2)
*/
