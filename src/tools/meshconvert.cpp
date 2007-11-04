/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdarg>
#include <vector>

#include "meshconvert.h"
#include "logging.h"
#include "compat.h"

using namespace std;
using std::string;

// directory separator
#ifdef WIN32

#define PLATFORM_SLASH '\\'

#else

#define PLATFORM_SLASH '/'

#endif

////////////////////// global data - yuck

UVMap *uvs = NULL;
long num_uvs; // TODO: == hdr.num_verts?

VHotObj *vhots = NULL;
Vertex *vertices = NULL;
MeshMaterial *materials = NULL;
MeshMaterialExtra *materialsExtra = NULL;
SubObjectHeader *objects = NULL;

char *meshOutPath = NULL;
char *materialOutPath = NULL;
char *fileName = NULL;
char *fileBaseName = NULL;

// HERE come the resulting structures we are desperately trying to fill:
short maxslotnum = 0;
short *slot2matnum = NULL; // convert slot number to the material index...

vector< SingleMaterialMesh* > outputters;

/////////////////////////////////////////// HELPER FUNCTIONS

// write out the material file
/*
The structure we use is:
material basename/matNN
{
	technique
	{
		pass
		{
			ambient x y z
			diffuse x y z

			texture_unit
			{
				texture filename
			}
		}
	}
}
*/
void SaveMaterialFile(char* basename, char *path, BinHeader &hdr) {
	char filepath[1024];

	snprintf(filepath, 1024, "%s%s.material", path, basename);
	log_info("Saving material file to %s", filepath);

	ofstream out(filepath, ios::binary);

	int n;

	for (n = 0; n < hdr.num_mats; n++) {
		out << "material " << basename << "/" << "Mat" << n << endl;
		out << "{" << endl; // material start
		out << "	technique" << endl;
		out << "	{" << endl; // technique start

		out << "		pass" << endl;
		out << "		{" << endl; // pass start

		float transp = -1;
		float illum = 0;

		// if the Transparency / illumination is present, output those too
		if (hdr.mat_flags & MD_MAT_TRANS) {
			transp = materialsExtra[n].trans;
		}

		if (hdr.mat_flags & MD_MAT_ILLUM) {
			illum = materialsExtra[n].illum;
		}
		// put the material specifications out
		if (materials[n].type == MD_MAT_COLOR) {
			// the material descriptions for textureless material
			// we give this a ambient, difuse specular parameters
			log_debug("		Material %d is MAT_COLOR", n);
			ostringstream colorstr;

			colorstr << ((float)materials[n].colour[2])/255 << " ";
			colorstr << ((float)materials[n].colour[1])/255 << " ";
			colorstr << ((float)materials[n].colour[0])/255;

			if (transp > 0)
				colorstr << " " << transp;

			// TODO: think off a better values maybe?
			out << "\t\t\tambient " << "0 0 0" << endl;
			out << "\t\t\tdiffuse " << colorstr.str() << endl;
			out << "\t\t\tspecular " << illum << " " << illum << " " << illum << endl;
			// enda line
			out << endl;

		} else if (materials[n].type == MD_MAT_TMAP) {
			log_debug("		Material %d is MAT_TMAP", n);
			// the material descriptions for textured material
			/*
			TODO: if the mesh comes weird, use some of these settings
			out << "\t\t\tambient" << "0 0 0" << endl;
			out << "\t\t\tdiffuse" << "1 1 1" << endl;*/

			out << "\t\t\ttexture_unit" << endl << "\t\t\t{" << endl;
			out << "\t\t\t\ttexture " << materials[n].name << endl;
			out << "\t\t\t}" << endl;

		} else
			log_error("Unknown material (%d) type : %d", n, materials[n].type);



		out << "		}" << endl; // pass end
		out << "	}" << endl; // technique end
		out << "}" << endl << endl; // material end
	}

	out.close();
}

//////////
int SlotToMatIndex(short slot) {
	if (slot < 0)
		return -1;

	if (slot > maxslotnum)
		return -1;

	return slot2matnum[slot];
}

void addTriangle(BinHeader &hdr, SubObjectHeader &shdr, int objidx, unsigned char material,
			short ida, short idb, short idc,
			short uva, short uvb, short uvc) {

	log_verbose("\t\t\t\tAdding triangle : material %d, vertex indices %d, %d, %d (UV: %d, %d, %d)", material, ida, idb, idc, uva, uvb, uvc);


	if (material >= hdr.num_mats) {
		log_error("Material %d is out of range");
		return;
	}

	outputters[material]->addTriangle(objidx, ida, idb, idc, uva, uvb, uvc);
}

void addTriangle(BinHeader &hdr, SubObjectHeader &shdr, int objidx, unsigned char material,
			short ida, short idb, short idc) {

	log_verbose("\t\t\t\tAdding no-tex triangle : material %d, vertex indices %d, %d, %d", material, ida, idb, idc);

	if (material >= hdr.num_mats) {
		log_error("Material %d is out of range");
		return;
	}

	outputters[material]->addTriangle(objidx, ida, idb, idc);
}

// the V6 direct triangle index list...
// This is not a guranteed thing, it seems that a study of V6 format is needed
void LoadDirectTriList(ifstream &in, BinHeadType &thdr, BinHeader &hdr, BinHeader2 &hdr2, int objidx, SubObjectHeader &shdr) {
	in.seekg(hdr2.offset_parts, ios::beg);

	PolyPartsShorts *parts;
	parts = new PolyPartsShorts[num_uvs];
	in.read((char *)parts, num_uvs * sizeof(PolyPartsShorts));

	// iterate through the triangles and add them all to the output buffers.
	// I don't quite understand where to get the material number for this version of Mesh
	// maybe there is a list of materials per triangle after the parts offset, we'll see
	int n;

	for (n = 0; n < num_uvs; n++)
		addTriangle(hdr, shdr, objidx, 0, parts[n].a, parts[n].b, parts[n].c, parts[n].a, parts[n].b, parts[n].c);

	delete[] parts;
}

void loadPolygon(ifstream &in, BinHeadType &thdr, BinHeader &hdr, BinHeader2 &hdr2, int objidx, SubObjectHeader &shdr, long offset) {
	log_verbose("\t\t\t\tloading polygon on offset %04X : ", hdr.offset_pgons + offset);
	int oldpos = in.tellg();

	ObjPolygon polyHdr;

	in.seekg(hdr.offset_pgons + offset, ios::beg);
	in.read((char *) &polyHdr, sizeof(ObjPolygon));


	short *vertex_indices;
	short *normal_indices; // ??
	short *uv_indices = NULL;

	vertex_indices = new short[polyHdr.num_verts];
	normal_indices = new short[polyHdr.num_verts];
	in.read((char *) vertex_indices, sizeof(short) * polyHdr.num_verts);
	in.read((char *) normal_indices, sizeof(short) * polyHdr.num_verts);

	if ( polyHdr.type == MD_PGON_TMAP ) {
		log_verbose("\t\t\t\tTextured, reading indices");
		uv_indices = new short[polyHdr.num_verts];
		in.read((char *) uv_indices, sizeof(short) * polyHdr.num_verts);
	}

	char Material;

	if ( thdr.version == 4 )
		in.read(&Material, 1);
	else {
		int mat = SlotToMatIndex(polyHdr.data); // Material here uses a slot... we'll se if the v4 does that too (so no -1)
		log_verbose("\t\t\t\tMaterial, type, textured : %d %02X %d", mat, polyHdr.type, (polyHdr.type == MD_PGON_TMAP));

		if ((mat < 0) || (mat >= hdr.num_mats)) {
			log_error("\t\t\t\tInvalid material number %d, not adding the polygon (slot %d)", mat, polyHdr.data);
			return;
		}

		Material = mat;
	}

	// now let's triangularize
	// we allways have N-2 triangles per polygon (3 -> 1, 4 -> 2... etc)
	for (int i = 0; i < polyHdr.num_verts - 1; i++) {
		// depending on texturization
		if ( polyHdr.type == MD_PGON_TMAP )
			addTriangle(hdr, shdr, objidx, Material,
				vertex_indices[0],
				vertex_indices[i+1],
				vertex_indices[i],
				uv_indices[0],
				uv_indices[i+1],
				uv_indices[i]);
		else {
			if (materials[Material].type == MD_MAT_TMAP) {
				log_error("Material needs UV and none found, using vertex index for uv");
				addTriangle(hdr, shdr, objidx, Material,
					vertex_indices[0],
					vertex_indices[i+1],
					vertex_indices[i],
					vertex_indices[0],
					vertex_indices[i+1],
					vertex_indices[i]);
			} else
				addTriangle(hdr, shdr, objidx, Material,
					vertex_indices[0],
					vertex_indices[i+1],
					vertex_indices[i]);
		}
	}

	// release the structures
	delete[] vertex_indices;
	delete[] normal_indices;
	if (uv_indices != NULL)
		delete[] uv_indices;

	// return to the old position
	in.seekg(oldpos, ios::beg);
}

void parseSubNode(ifstream &in, BinHeadType &thdr, BinHeader &hdr, BinHeader2 &hdr2, int objidx, SubObjectHeader &shdr, long offset) {
	char splittype;
	short polys[1024];
	int polycount;

	log_debug("\t\t\tSub-node %d on offset %04lX", offset, hdr.offset_nodes + offset);
	in.seekg(hdr.offset_nodes + offset, ios::beg);
	in.read(&splittype, 1);

	NodeRaw     nr;
	NodeCall    nc;
	NodeSplit   ns;

	switch (splittype) {
		case MD_NODE_HDR:
				NodeHeader ndhdr;
				in.read((char *) &ndhdr, sizeof(NodeHeader));
				log_debug("\t\t\t-- Header node --");
				log_debug("\t\t\t * Node values: [flag, objnum, unk1]: %d %d %d", (int)ndhdr.flag, (int)ndhdr.object_number, (int)ndhdr.c_unk1);

				parseSubNode (in, thdr, hdr, hdr2, objidx, shdr, offset + sizeof(NodeHeader));
			break;
		case MD_NODE_SPLIT:
				log_debug("\t\t\t-- Split node --");
				in.read((char *) &ns, sizeof(NodeSplit));
				// the polygons are read sequentially, and processed

				polycount = ns.pgon_before_count + ns.pgon_after_count;
				in.read((char *) polys, sizeof(short) * polycount);

				log_debug("\t\t\tChilds : %d - %d", ns.behind_node, ns.front_node);

				log_verbose("\t\t\tPolygon count: %d", polycount);

				if (ns.behind_node < offset)
					log_error("\t\t\tSplit to a lower node number - behind node!");
				else
				if ( ( ns.behind_node >= shdr.node_start ) ) { // todo:  && ( ns.behind_node < NodeMax )
					parseSubNode (in, thdr, hdr, hdr2, objidx, shdr, ns.behind_node);
				}

				if (ns.behind_node < offset)
					log_error("\t\t\tSplit to a lower node number - front node!");
				else
				if ( ( ns.front_node >= shdr.node_start ) ) { // todo: && ( ns.front_node < NodeMax )
					parseSubNode (in, thdr, hdr, hdr2, objidx, shdr, ns.front_node);
				}

				for (int n = 0; n < polycount; n++)
					loadPolygon(in, thdr, hdr, hdr2, objidx, shdr, polys[n]);

				break;

		case MD_NODE_CALL:
				log_debug("\t\t\t-- Call node --");
				in.read((char *) &nc, sizeof(NodeCall));
				// the polygons are read sequentially, and processed

				polycount = nc.pgon_before_count + nc.pgon_after_count;
				log_verbose("\t\t\tPolygon count: %d", polycount);

				in.read((char *) polys, sizeof(short) * polycount);
				for (int n = 0; n < polycount; n++)
					loadPolygon(in, thdr, hdr, hdr2, objidx, shdr, polys[n]);

			break;

		case MD_NODE_RAW:
				log_debug("\t\t\t-- Raw node --");
				in.read((char *) &nr, sizeof(NodeRaw));
				// the polygons are read sequentially, and processed

				in.read((char *) polys, sizeof(short) * (nr.pgon_count));
				log_verbose("\t\t\tPolygon count: %d", nr.pgon_count);

				for (int n = 0; n < nr.pgon_count; n++)
					loadPolygon(in, thdr, hdr, hdr2, objidx, shdr, polys[n]);

			break;

		default: log_error("Unknown node type %d at offset %04lX", splittype,  hdr.offset_nodes + offset);
	}
}

void LoadTreeNodeGeometry(ifstream &in, BinHeadType &thdr, BinHeader &hdr, BinHeader2 &hdr2, int objidx, SubObjectHeader &shdr) {

	parseSubNode(in, thdr, hdr, hdr2, objidx, shdr, shdr.node_start);
}

/* Processes object list, and should prepare the global vertex buffer, index buffer, skeleton tree, vertex - joint mapping
*/
void ProcessObjects(ifstream &in, BinHeadType &thdr, BinHeader &hdr, BinHeader2 &hdr2) {
	objects = new SubObjectHeader[hdr.num_objs];

	in.seekg(hdr.offset_objs, ios::beg);
	in.read((char *) objects, sizeof(SubObjectHeader) * hdr.num_objs);

	// TODO: I do not have other choice, than to make the skeleton so the VHOTS in the sub-objects are made as bones from the joint point to the vhot.
	// This is because the ogre does not have anything like the VHOT vertices... (This should not be a serious problem, I hope - they only should have compatible naming - VHOT1 for example)

	for (int x = 0; x < hdr.num_objs; x++) {
		// logging the object names:
		log_debug("!!!    - sub-object name : %s", objects[x].name);
		log_debug("       	- movement type : %d", (int) objects[x].movement);

		// the sub-object readout. We directly process the geometry too, and build the constructs needed to export the file

		// The v. 6 file has a direct list of triangle vertex and uv indices, the others have a BSP structured data
		// we call the add Triangle function on the resulting triangle anyway...
		if (thdr.version == 6) {
			LoadDirectTriList(in, thdr, hdr, hdr2, x, objects[x]);
		} // but the pre - 6 versions do have a tree specifying the geometry
			else
			LoadTreeNodeGeometry(in, thdr, hdr, hdr2, x, objects[x]);
	}

	// now the important stuff...

	// the first object is allways root, and has a bogus transformation...
	// the others have a good transformation info, and the connection of skeleton bones seems to be dependent on the sub-object names

}


/////////////////////////////////////////// Global processing functions
// The object model type of mesh
void readObjectModel(ifstream &in, BinHeadType &thdr) {
	BinHeader hdr;
	BinHeader2 hdr2;

	log_info("LGMD mesh processing - e.g. an object");
	long size = 0;

	log_info("Header version : %d", thdr.version);

	// determine the header size depending on the version of the mesh
	switch ((int) thdr.version) {
		case 3: size = SIZE_BIN_HDR_V3; break;
		case 4: size = SIZE_BIN_HDR_V4; break;
		case 6: size = SIZE_BIN_HDR_V6; break;
		default: size = -1; // TODO: fixfix
	}

	if (size <= 0) {
		log_fatal("FATAL: The object mesh has an unsupported version : %d", thdr.version);
		return;
	}

	// erase the header
	memset((char *) &hdr, 0, sizeof(BinHeader));

	// read the model header
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

	log_info(" * Materials (%d)", hdr.num_mats);
	log_debug("  - offset %06lX",hdr.offset_mats);

	// Materials
	materials = new MeshMaterial[hdr.num_mats];

	// this is not good! we read right through other tables...
	in.seekg(hdr.offset_mats, ios::beg);

	in.read((char *) materials, hdr.num_mats * sizeof(MeshMaterial));

	for (int x = 0; x < hdr.num_mats; x++) {
		// logging the material names:
		log_debug("       - material name : %s (type: %04X, slot: %d)", materials[x].name, materials[x].type, materials[x].slot_num);

		// computing the maximal slot number
		if (materials[x].slot_num > maxslotnum)
			maxslotnum = materials[x].slot_num;

		// intialise the outputter
		SingleMaterialMesh *ins = new SingleMaterialMesh(fileBaseName, x, (materials[x].type == MD_MAT_TMAP));

		if (ins == NULL)
			log_error("Material %d failed to construct", x);

		outputters.push_back(ins);
	}

	// slot to index material conversion table preparation
	if (thdr.version == 3) { // it seems only v3 meshes use slots for materials
		slot2matnum = new short[maxslotnum + 1];
		for (int x = 0; x < maxslotnum; x++)
			slot2matnum[x] = -1;

		for (int x = 0; x < hdr.num_mats; x++) {
			log_debug("Adding slot %d (-> %d) to slot2matnum", materials[x].slot_num, x);
			slot2matnum[materials[x].slot_num] = x;
		}
	}

	// if we need extended attributes
	if ( hdr.mat_flags & MD_MAT_TRANS || hdr.mat_flags & MD_MAT_ILLUM ) {
		log_info(" * Extra materials (%d)", hdr.num_mats);
		log_debug("  - actual offset %06lX", (int) in.tellg());
		// Extra Materials
		materialsExtra = new MeshMaterialExtra[hdr.num_mats];
		in.read((char *) materialsExtra, hdr.num_mats * sizeof(MeshMaterialExtra));
	}

	// Read the UV map vectors
	log_info(" * UVMAP (%d / %d)", hdr.num_verts, num_uvs);
	log_debug("  - offset %06lX",hdr.offset_uv);
	// prepare and read uvmaps
	uvs = new UVMap[num_uvs];

	in.seekg(hdr.offset_uv, ios::beg);
	// I have to rely on shadowspawn here: read uvs in the number of verts - those are differing, but I guess that this is solvable later on
	// After looking into the binary, the num_uvs calculated seem to be reasonable
	// in.read((char *) uvs, hdr.num_verts * sizeof(UVMap));
	in.read((char *) uvs, num_uvs * sizeof(UVMap));

	// TODO: shadowspawn reverses the U part of pre-6 version mesh UV table here. See if we need that too


	// VHOT:
	log_info(" * VHOT (%d)", hdr.num_vhots);
	log_debug("  - offset %06lX",hdr.offset_vhots);

	if (hdr.num_vhots > 0) {
		// prepare and read the vhots
		vhots = new VHotObj[hdr.num_vhots];
		in.seekg(hdr.offset_vhots, ios::beg);
		in.read((char *) vhots, hdr.num_vhots * sizeof(VHotObj));
	} else {
		vhots = NULL;
		log_info("No vhots in this model");
	}
	// Vertex table....
	log_info(" * Vertices (%d)", hdr.num_verts);
	log_debug("  - offset %06lX",hdr.offset_verts);

	// prepare and read the vertices
	vertices = new Vertex[hdr.num_verts];
	in.seekg(hdr.offset_verts, ios::beg);
	in.read((char *) vertices, hdr.num_verts * sizeof(Vertex));

	log_debug("Setting important data structures in outputters...");

	log_info("Object tree processing:");

	SaveMaterialFile(fileBaseName, "materials/", hdr);
	ProcessObjects(in, thdr, hdr, hdr2);

	for (int x = 0; x < hdr.num_mats; x++) {
		outputters[x]->setObjects(objects, hdr.num_objs);
		outputters[x]->setVertices(vertices, hdr.num_verts);
		outputters[x]->setUVMaps(uvs, num_uvs);
	}

	// final output
	char filepath[2048];

	snprintf(filepath, 1024, "%s%s.xml", meshOutPath, fileBaseName); // TODO: Path!

	ofstream ofs(filepath);

	ofs << "<mesh>" << endl << "\t<submeshes>" << endl;

	for (int x = 0; x < hdr.num_mats; x++)
		outputters[x]->output(ofs, "\t\t");


	ofs << "\t</submeshes>" << endl << "</mesh>" << endl;

	ofs.close();

	// cleanout
	log_info("Releasing used pointers");
	log_debug(" * outputters");
	for (int x = 0; x < hdr.num_objs; x++)
		if (outputters[x] != NULL) {
			log_debug("   - %d", x);
			//SingleMaterialMesh *m = outputters[x];
			//delete m; // TODO: Why oh why is this causing segfaults?
		}

	log_debug(" * vhot");
	if (vhots != NULL)
		delete[] vhots;

	log_debug(" * vert");
	if (vertices != NULL)
		delete[] vertices;

	log_debug(" * vert");
	if (uvs != NULL)
		delete[] uvs;

	log_debug(" * materials");
	if (materials != NULL)
		delete[] materials;

	log_debug(" * extras");
	if (materialsExtra != NULL)
		delete[] materialsExtra;

	log_debug(" * objects");
	if (objects != NULL)
		delete[] objects;

	log_debug(" * slot2matnum");

	if (slot2matnum != NULL)
		delete[] slot2matnum;

	// the end
	log_info("all done");
}

void tidy() {
	if (meshOutPath != NULL)
		delete[] meshOutPath;

	if (materialOutPath != NULL)
		delete[] materialOutPath;

	if (fileBaseName != NULL)
		delete[] fileBaseName;
}

// prepare some global variables depending on the commandline shape
// we understand these:
// -x output dir - XML (including the trailing slash)
// -m output dir - mesh
void displayHelpAndExit() {
	cout << "meshconverter [-x XML-out-path] [-m Material-out-path] [-l loglevel] filename" << endl;
	cout << "Loglevel : 0 - fatal, 1 - error, 2 - info, 3 - debug 4-all" << endl;
	cout << "Platform slash : '" << PLATFORM_SLASH << "'" << endl;
	tidy();
	exit(1);
}

void putTrailingSlash(char **string) {
	if (strlen(*string) > 0) {
		char endc = (*string)[strlen(*string)-1];

		if (endc != PLATFORM_SLASH) {// no trailing slash!

			char *newstring = new char[strlen(*string) + 2];

			sprintf(newstring, "%s%c", *string, PLATFORM_SLASH); // string allocation - aaw the plain C

			delete[] *string;
			*string = newstring;
		}
	}
}

void parseCmdLine(int argc, char* argv[]) {
	int pos = 0;

	fileName = NULL;

	meshOutPath = new char[2];
	materialOutPath = new char[2];
	strcpy(meshOutPath, "");
	strcpy(materialOutPath, "");

	while (++pos < argc) {
		if (argv[pos][0] == '-') { // an option switch
			if (strlen(argv[pos]) == 2) {
				switch (argv[pos][1]) {
					case 'x' :
						delete[] meshOutPath;
						meshOutPath = new char[strlen(argv[++pos]) + 1];
						strcpy(meshOutPath, argv[pos]);
						break;
					case 'm' :
						delete[] materialOutPath;
						materialOutPath = new char[strlen(argv[++pos]) + 1];
						strcpy(materialOutPath, argv[pos]);
						break;
					case 'l' :
						++pos;

						if (strlen(argv[pos]) > 1)
							log_error("Non-fatal: unknown loglevel %s",argv[pos]);
						else {
							int level = argv[pos][0] - '0';
							if ( level > LOG_VERBOSE )
								level = LOG_DEBUG;
							if ( level < 0 )
								level = 0;

							log_info("Setting loglevel to %d", level);
							loglevel = level;
						}
						break;
					default:
						log_error("Option %s not understood", argv[pos]);
						displayHelpAndExit();
				}
			} else {
				log_error("Option %s not understood", argv[pos]);
				displayHelpAndExit();
			}
		} else {
			log_debug("Argument %s (%c) is a filename", argv[pos], argv[pos][0]);
			fileName = argv[pos];
		}


		// if the params were not included
		if (pos >= argc) {
			log_error("Some option has unsatisfied parameters (option without value)");
			displayHelpAndExit();
		}
	}

	if (fileName==NULL)
		displayHelpAndExit();

	// fix the output path strings
	putTrailingSlash(&meshOutPath);
	putTrailingSlash(&materialOutPath);


	// cut - out the base name of the input file, and save it into the filebasename
	char helper[255];
	strncpy ( (char *) helper, fileName, 254);

	char *tp = strrchr ( (char *) helper, '.' );

	if ( tp )
		*tp = 0x0;

	char *bname = strrchr ( (char *) helper, PLATFORM_SLASH );

	if ( bname )
			bname++;  // spit out the slash
		else
			bname = helper;

	fileBaseName = new char[strlen(bname) + 1];
	strcpy(fileBaseName, bname);
}

int main(int argc, char* argv[]) {
	cout << "MeshConvert (.bin to .xml converter for all kinds of LG meshes)" << endl;

	parseCmdLine(argc, argv);

	// open the input bin mesh file
	BinHeadType header;

	ifstream input(fileName, ios::binary);
	cout << "Opened " << fileName << endl;

	input.read((char *) &header, sizeof(header));

	if (strncmp(header.ID,"LGMD",4) == 0) {
		readObjectModel(input, header);
	} else
	if (strncmp(header.ID,"LGMM",4) == 0) {
		cout << "AI mesh type - Unimplemented for now" << endl;
	}	else
		cout << "Unknown type of file : " << argv[1] << endl;


	input.close();

	tidy();
	return 0;
}
