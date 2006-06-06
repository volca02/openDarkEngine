#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include "meshconvert.h"

using namespace std;
using std::string;
//////////////////// logging
#define LOG_DEBUG 3
#define LOG_INFO 2
#define LOG_ERROR 1
#define LOG_NONE 0

long loglevel = LOG_DEBUG;

void log(long level, string s) {
	if (loglevel <= level)
		cerr << s;
}

void log_error(string s) {
	log(LOG_ERROR, s);
}

void log_info(string s) {
	log(LOG_INFO, s);
}

void log_debug(string s) {
	log(LOG_DEBUG, s);
}

////////////////////// global data - yuck

UVMap *uvs;
long num_uvs; // TODO: == hdr.num_verts?

VHotObj *vhots;
Vertex *vertices;

void readModelHeaderTables(ifstream &in, BinHeader &hdr, BinHeader2 &hdr2, BinHeadType &thdr) {
	long size = 0;
	
	switch (thdr.version) {
		case 3: size = SIZE_BIN_HDR_V3; break;
		case 4: size = SIZE_BIN_HDR_V4; break;
		case 6: size = SIZE_BIN_HDR_V6; break;
		default: size = -1; // TODO: fixfix
	}
	
	in.read((char *) &hdr, size);
	
	// we should definetaly complete the header data depending on the version somehow...
	
	if (thdr.version == 6) { // we have another header to look at
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
	}
	
	// prepare and read uvmaps
	uvs = new UVMap[num_uvs];
	
	in.seekg(hdr.offset_uv, ios::beg);
	// in.read((char *) uvs, num_uvs * sizeof(UVMap));
	// I have to rely on shadowspawn here: read uvs in the number of verts (maybe these equal anyways)
	in.read((char *) uvs, hdr.num_verts * sizeof(UVMap));
	
	// TODO: shadowspawn reverses the U part of pre-6 version mesh UV table here
	
	// prepare and read the vhots
	vhots = new VHotObj[hdr.num_vhots];
	in.seekg(hdr.offset_vhots, ios::beg);
	in.read((char *) vhots, hdr.num_vhots * sizeof(VHotObj));
	
	// prepare and read the vertices
	vertices = new Vertex[hdr.num_verts];
	in.seekg(hdr.offset_verts, ios::beg);
	in.read((char *) vertices, hdr.num_verts * sizeof(Vertex));
}

int main(int argc, char* argv[]) {
	cout << "meshconvert\n";


	// open the input bin mesh file
	BinHeadType header;
	ifstream input("apple.bin", ios::binary); 
	
	input.read((char *) &header, sizeof(header));
	input.close();
	
	if (strncmp(header.ID,"LGMD",4) == 0) {
		cout << "Model type\n";
		BinHeader binhdr;
		BinHeader2 binhdr2;
		readModelHeaderTables(input, binhdr, binhdr2, header);
		
	} else 
	if (strncmp(header.ID,"LGMM",4) == 0) {
		cout << "AI mesh type\n";
	}
		else printf("Unknown type");
	return 0;
}
