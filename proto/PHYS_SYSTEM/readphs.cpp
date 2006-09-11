#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define uint8 uint8_t
#define sint8 int8_t
#define uint16 uint16_t
#define sint16 int16_t
#define sint32 uint32_t
#define uint32 int32_t

typedef struct { // SIZE: 8
	uint32	phys_version;
	uint32	obj_count;
} phys_hdr;


typedef struct { // SIZE: 12
	float	x;
	float   y;
	float	z;
} t_coord;

// Vector with length/norm (Velocities use this)
typedef struct { // SIZE: 16
	float	x;
	float   y;
	float	z;
	float	norm;
} t_coord_norm;

// Once I'll understand what the sub-objects are, I'll work this out :
typedef struct {
	t_coord pos1;
	sint32  unk1;
} t_subObject;

const char hchr[]="0123456789ABCDEF";
////////////////// HELPERS //////////////////
void p_coord(t_coord &c) {
	printf("		[ %+10.2f %+10.2f %+10.2f ]\n",c.x,c.y,c.z);
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
	printf("\tObject Count : %d\n", hdr.obj_count);
}

// Bounding volume type printout. There is a difference for 0,2,4 - sphere hat for example
bool p_BVolType(uint32 bv) {
	bool err = false;
	
	printf("\tBounding [%1d]   : ", bv);
	switch (bv) {
		case 0:
		case 2:
		case 4:
			printf("Sphere");
			break;
		case 1:
			printf("BSP Tree - Object defined boundry");
			break;
		case 3:
			printf("BOX");
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
			case 'x':
				uint32 d;
				fread(&d,1,4,f);
				printf("%8XX ",d);
				break;
			default:
				printf("?(%c)", format[x]);
		}
	}
}


// This should read the bounding volume definitions
void readBoundingDefinition(FILE *f, uint32 btype, uint32 version) {
	printf("\tBounding Volume definition   :\n");
	switch (btype) {
		case 0:
		case 2:
		case 4:
			float radius;
			printf("\tBOUNDING UNKNOWN  : "); readStruct("FFFFFFFFFFFFFFFFF", f); printf("\n");
			fread(&radius,4,1,f);
			printf("\tSphere  - Radius %8.2g\n", radius);
			// printf("\tBOUNDING UNKNOWN  : "); readStruct("FFF", f); printf("\n");
			break;
		case 1:
			printf("BSP Tree - Object defined boundry (no data)\n");
			break;
		case 3:
			printf("\tBOX : ");
			readVector3(f);
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
	uint32 bvolume;
	fread(&bvolume,1,4,f);
	if (p_BVolType(bvolume))
		return false;
	
	// 
	uint32 object_id, num_subobjs, count2;
	fread(&object_id,1,4,f);
	fread(&num_subobjs,1,4,f);
	fread(&count2,1,4,f);
	printf("\tObjectID      : %d\n",object_id);
	printf("\tSub-Objects   : %d\n",num_subobjs);
	printf("\tFlags         : %08X\n",count2);
	// Will have to investigate what the flags mean. It seems there are flags for doors and translational objects, rotational objects, etc
	
	float gravity;
	fread(&gravity,1,4,f);
	printf("\tGravity       : %f\n", gravity);
	
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
	Following are structures of 18 floats/longs per line, the count seems to differ per bounding volume type (But I didn't manage to find what the counts are yet)
	BOX has 7... SPHERE has 2 (Seems to be always SubObject Count + 1)
	The first 3 floats seem to mean the position of a certain collision element. For box, those are the centers of the sides.
	The last element is set to the center of the Collision Volume.
	*/
	for (int n = 0; n < num_subobjs + 1; n++) {
		printf("\t SUBOBJ[%4d] : ",n); readStruct("FFFLFFFFXFFFFFLFXF", f);printf("\n");
	}
	
	
	fpos(f);
	printf("\tCOG Offset : "); readVector3(f);
	
	// Sub Object direction Vectors (translations from the center)
	fpos(f);
	for (int n = 0; n < num_subobjs; n++) {
		printf("\tSubObj Translation vec. [%6d] : ", n); readVector3(f);
	}
	fpos(f);
	
	
	// I think that this is the place where the position and speed (rot+trans) are defined
	// The size of all this is somewhat releted to some previous values. For example ca and cb could hold the length of the following block
	// But how? 
	
	// The following block is not understood yet. seems to be parametrized by the subobj count too.
	/* 132 bytes for cube (4), 252 bytes for sphere (7)*/
	printf("\t???      : "); readStruct("FFLLLLLL", f);printf("\n");
	
	fpos(f);
	// Translation speed here
	printf("\tVelocity : "); readVector3(f);
	
	printf("\tUnknown  : "); readStruct("LLLLLLL", f);printf("\n");
	
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
	
	
	// At the end. There will be rotations (and speeds?) for all axises, if there is any(?) - investigate by switching all off
	printf("\t???      : "); readStruct("XLLX", f);printf("\n");
	printf("\t???      : "); readStruct("FL", f);printf("\n");
	
	// This seems to be bounded with the rot_flags or rest_flags, the bvolume comparision is just a hack to let me read past this data
	// I switch one rot axis off, and the data size here does not change. So can be based on rest axises, or some nonzeroness of those
	if (bvolume == 0) {
		printf("\tUnknown  : "); readStruct("LLFFFFFFFF", f);printf("\n");
		printf("\tUnknown  : "); readStruct("LLFFFFFFFF", f);printf("\n");
		printf("\tUnknown  : "); readStruct("LFFFFFFFFF", f);printf("\n");
	}
		
	fpos(f);
	
	// Flags for the Controls Follow (BOX offset: 0x374, Sphere offset: 0x20c)
	uint32 control_flags;
	fread(&control_flags,1,4,f);
	
	/// Control flags
	/*
	8 - rotation
	*/
	printf("\tControl Flags : %08X\n", control_flags);
	
	printf("\tUnknown  : "); readStruct("FFFF", f); printf("\n");
	// Now the control vectors (t_coord_norm * 4?)
	printf("\tAxis Velocity : "); readVector3N(f);
	printf("\tVelocity      : "); readVector3N(f);
	printf("\tRot. Velocity : "); readVector3N(f);
	fpos(f);
	
	// I think this could be a rotation vector... It was nonzero for moving object
	printf("\tUnk. Vector 1  : "); readVector3(f);
	
	// We should be on the right offset for the bounding volume definition readout
	fpos(f);

	readBoundingDefinition(f, bvolume, version);

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
	
	// Do only one for now, we do not know how to get the lenght right
	//for (int n = 0; n < 1; n++) {
	
	for (int n = 0; n < hdr.obj_count; n++) {
		if (!readObjectPhys(f, n, hdr.phys_version))  {
			fprintf(stderr,"Error encountered, ending readout\n");
			break;
		}
	}
	
	// This is the place where I will have to start reading contacts. Those should be after the phys object list i think.
	
	fclose(f);
	
}
