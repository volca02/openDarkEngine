#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdarg>

#include "meshconvert.h"
#include "logging.h"

using namespace std;
using std::string;


////////////////////// global data - yuck

UVMap *uvs = NULL;
long num_uvs; // TODO: == hdr.num_verts?

VHotObj *vhots = NULL;
Vertex *vertices = NULL;
MeshMaterial *materials = NULL;
MeshMaterialExtra *materialsExtra = NULL;
SubObjectHeader *objects = NULL;

char *fileBaseName = NULL;
// HERE come the resulting structures we are desperately trying to fill:
// vector<Vertex> out_vertices;


/////////////////////////////////////////// HELPER FUNCTIONS
/**
Recalc the vertex coordinates using the given transformation;
*/
Vertex transformVertex(Vertex &in, SubObjTransform &trans) {
	Vertex out;
	
	out.x = in.x * trans.f[0] + in.y * trans.f[3] + in.z * trans.f[6] + trans.AxlePoint.x;
	out.y = in.x * trans.f[1] + in.y * trans.f[4] + in.z * trans.f[7] + trans.AxlePoint.y;
	out.z = in.x * trans.f[2] + in.y * trans.f[5] + in.z * trans.f[8] + trans.AxlePoint.z;
	
	return out;
}

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
		out << "material " << basename << "/" << "mat" << n << endl;
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

/* Processes object list, and should prepare the global vertex buffer, index buffer, skeleton tree, vertex - joint mapping
*/
void ProcessObjects(ifstream &in, BinHeadType &thdr, BinHeader &hdr, BinHeader2 &hdr2) {
	objects = new SubObjectHeader[hdr.num_objs];
	
	in.seekg(hdr.offset_objs, ios::beg);
	in.read((char *) objects, sizeof(SubObjectHeader) * hdr.num_objs);
	
	// TODO: I do not have other choice, than to make the skeleton so the VHOTS in the sub-objects are made as bones from the joint point to the vhot.
	// This is because the ogre does not have anything like the VHOT vertices... (This should not be a serious problem, I hope - they only should have compatible numbering	)
	
	XMLOgreMesh m();
	
	for (int x = 0; x < hdr.num_objs; x++) {
		// logging the object names:
		log_debug("       - sub-object name : %s", objects[x].name);
		log_debug("       	- movement type : %d", (int) objects[x].movement);
		
		/*log_debug("Transform : ");
		if (loglevel >= LOG_DEBUG)
			cout << objects[x].trans << "\n";*/
		
		// the sub-object readout. We directly process the geometry too, and build the constructs needed to export the file
	}
	
	// now the important stuff...

	// the first object is allways root, and has a bogus transformation...
	// the others have a good transformation info, and the connection of skeleton bones seems to be dependent on the sub-object names
	
	/*m.setVertexListPtr(vertices);
	m.setMaterialListPtr(materials);
	m.setExtraMaterialListPtr(materialsExtra);
	m.setObjectList(objects);*/
}


/////////////////////////////////////////// Global processing functions
// The object model type of mesh
void readObjectModel(ifstream &in, BinHeadType &thdr) {
	BinHeader hdr;
	BinHeader2 hdr2;
	
	log_info("LGMD mesh processing - e.g. an object");
	long size = 0;
	
	log_debug("Header version : %d", thdr.version);
	
	// determine the header size depending on the version of the mesh
	switch (thdr.version) {
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
	
	// logging the material names:
	for (int x = 0; x < hdr.num_mats; x++)
		log_debug("       - material name : %s", materials[x].name);
	
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
	// prepare and read the vhots
	vhots = new VHotObj[hdr.num_vhots];
	in.seekg(hdr.offset_vhots, ios::beg);
	in.read((char *) vhots, hdr.num_vhots * sizeof(VHotObj));
	
	// Vertex table....
	log_info(" * Vertices (%d)", hdr.num_verts);
	log_debug("  - offset %06lX",hdr.offset_verts);
	
	// prepare and read the vertices
	vertices = new Vertex[hdr.num_verts];
	in.seekg(hdr.offset_verts, ios::beg);
	in.read((char *) vertices, hdr.num_verts * sizeof(Vertex));
	
	log_info("Object tree processing:");
	
	SaveMaterialFile(fileBaseName, "materials/", hdr);
	ProcessObjects(in, thdr, hdr, hdr2);
	
	
	// cleanout
	log_info("Releasing used pointers");
	if (vhots != NULL)
		delete[] vhots;
	
	if (vertices != NULL)
		delete[] vertices;
	
	if (uvs != NULL)
		delete[] uvs;
	
	if (materials != NULL)
		delete[] materials;
	
	if (materialsExtra != NULL)
		delete[] materialsExtra;
	
	if (objects != NULL)
		delete[] materialsExtra;
	// the end
	log_info("all done");
}

int main(int argc, char* argv[]) {
	cout << "MeshConvert (.bin to .xml converter for all kinds of LG meshes)" << endl;


	// open the input bin mesh file
	BinHeadType header;
	
	if (argc < 2) {
		cout << "Specify a .BIN file as a parameter, please" << endl;
		return 1;
	}
	
	ifstream input(argv[1], ios::binary); 
	
	input.read((char *) &header, sizeof(header));
	
	
	/// I Know, I Know. This code below is a pure CRAP. For testing purposes only.
	char helper[255];
	strcpy ( (char *) helper, argv[1] );
	
	char *tp = strrchr ( (char *) helper, '.' );
	if ( tp )
		*tp = 0x0;
	
	fileBaseName = strrchr ( (char *) helper, '/' );
	
	if ( !fileBaseName )
			fileBaseName = helper;
		else
			fileBaseName++; // spit out the slash
	// --- END OF REALLY CRAP CODE (that does not mean that there is no crap code outside this section though ;) ) ---
		
	if (strncmp(header.ID,"LGMD",4) == 0) {
		readObjectModel(input, header);
	} else 
	if (strncmp(header.ID,"LGMM",4) == 0) {
		cout << "AI mesh type - Unimplemented for now" << endl;
	}	else 
		cout << "Unknown type" << endl;
	

	input.close();		
	return 0;
}
