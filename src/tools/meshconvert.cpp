#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include "meshconvert.h"

using namespace std;
using std::string;
//////////////////// logging
#define LOG_DEBUG 3
#define LOG_INFO 2
#define LOG_ERROR 1

#define LOG_SHOW_LEVEL false

long loglevel = LOG_DEBUG;

void log(long level, char *fmt, ...) {
	if (level > loglevel)
		return;
	
	va_list argptr;
	char result[255];
	
	va_start(argptr, fmt);
	vsnprintf(result, 255, fmt, argptr);
	va_end(argptr);
	
	printf("Log (%ld) : %s\n", level, result);
}

#define log_error(...) log(LOG_ERROR, __VA_ARGS__)
#define log_info(...) log(LOG_INFO, __VA_ARGS__)
#define log_debug(...) log(LOG_DEBUG, __VA_ARGS__)

////////////////////// global data - yuck

UVMap *uvs;
long num_uvs; // TODO: == hdr.num_verts?

VHotObj *vhots;
Vertex *vertices;
MeshMaterial *materials;
MeshMaterialExtra *materialsExtra;

void readModelHeaderTables(ifstream &in, BinHeader &hdr, BinHeader2 &hdr2, BinHeadType &thdr) {
	log_info("LGMD mesh processing");
	long size = 0;
	
	log_debug("Header version : %d", thdr.version);
	
	switch (thdr.version) {
		case 3: size = SIZE_BIN_HDR_V3; break;
		case 4: size = SIZE_BIN_HDR_V4; break;
		case 6: size = SIZE_BIN_HDR_V6; break;
		default: size = -1; // TODO: fixfix
	}
	
	in.read((char *) &hdr, size);
	
	// we should definetaly complete the header data depending on the version somehow...
	
	if (thdr.version == 6) { // we have another header to look at
		log_debug("V6 header");
		in.seekg(hdr.offset_hdr2, ios::beg);
		in.read((char *) &hdr2, sizeof(BinHeader2));
		
		// is this a right approach? I hope so
		hdr.offset_uv = hdr2.offset_uv;
		hdr.offset_verts = hdr2.offset_verts;
		hdr.offset_norms = hdr2.offset_norms;
		num_uvs = hdr2.num_uvs;
	} else {
		// It seems that pre-6 version headers do not contain number of uvs
		num_uvs = (hdr.offset_vhots - hdr.offset_uv) / sizeof (UVMap);
		
		log_debug("pre - V6 header, calculated num_uvs = %d", num_uvs);
	}
	
	log_info("Reading Tables:");
	
	log_info(" * UVMAP (%d / %d)", hdr.num_verts, num_uvs);
	// prepare and read uvmaps
	uvs = new UVMap[num_uvs];
	
	in.seekg(hdr.offset_uv, ios::beg);
	// in.read((char *) uvs, num_uvs * sizeof(UVMap));
	// I have to rely on shadowspawn here: read uvs in the number of verts (maybe these equal anyways)
	in.read((char *) uvs, hdr.num_verts * sizeof(UVMap));
	
	// TODO: shadowspawn reverses the U part of pre-6 version mesh UV table here
	
	log_info(" * VHOT (%d)", hdr.num_vhots);
	// prepare and read the vhots
	vhots = new VHotObj[hdr.num_vhots];
	in.seekg(hdr.offset_vhots, ios::beg);
	in.read((char *) vhots, hdr.num_vhots * sizeof(VHotObj));
	
	log_info(" * Vertices (%d)", hdr.num_verts);
	
	// prepare and read the vertices
	vertices = new Vertex[hdr.num_verts];
	in.seekg(hdr.offset_verts, ios::beg);
	in.read((char *) vertices, hdr.num_verts * sizeof(Vertex));
	
	log_info(" * Materials (%d)", hdr.num_mats);
	// Materials
	
	materials = new MeshMaterial[hdr.num_mats];
	materialsExtra = NULL;
	in.seekg(hdr.offset_mats, ios::beg);
	in.read((char *) materials, hdr.num_mats * sizeof(MeshMaterial));
	
	if ( hdr.mat_flags & MD_MAT_TRANS || hdr.mat_flags & MD_MAT_ILLUM ) {
		log_info(" * Extra materials (%d)", hdr.num_mats);
		// Extra Materials
		materialsExtra = new MeshMaterialExtra[hdr.num_mats];
		in.read((char *) materialsExtra, hdr.num_mats * sizeof(MeshMaterialExtra));
	}
	
	log_info("Object tree processing:");
}

int main(int argc, char* argv[]) {
	cout << "MeshConvert (.bin to .xml converter for all kinds of LG meshes)\n";


	// open the input bin mesh file
	BinHeadType header;
	ifstream input("apple.bin", ios::binary); 
	
	input.read((char *) &header, sizeof(header));
	
	
	if (strncmp(header.ID,"LGMD",4) == 0) {
		BinHeader binhdr;
		BinHeader2 binhdr2;
		readModelHeaderTables(input, binhdr, binhdr2, header);
		
	} else 
	if (strncmp(header.ID,"LGMM",4) == 0) {
		cout << "AI mesh type - Unimplemented for now\n";
	}
		else printf("Unknown type");
	

	input.close();		
	return 0;
}
