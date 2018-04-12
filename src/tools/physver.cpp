/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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

#include <iostream>

#include "File.h"
#include "FileGroup.h"
#include "integers.h"

using namespace Opde;

void usage(const char* message = NULL) {
    if (message)
	std::cerr << message << std::endl;

    std::cout << "physver File [files...]" << std::endl
	      << "  FILE - the dark database file to read" << std::endl;

    exit(1);
}

int typePhysVersion(const char* fname) {
	DarkFileGroup* gr = NULL;
	int rcode = 0;

	try {
		FilePtr src = FilePtr(new StdFile(fname, File::FILE_R));
		gr = new DarkFileGroup(src);

		FilePtr chunk = gr->getFile("PHYS_SYSTEM");

		uint32_t ver;
		chunk->readElem(&ver, sizeof(uint32_t));

		std::cerr << gr->getName() << ":" << ver << std::endl;
	} catch (...) {
		rcode = 1;
	}

	delete gr;
	return rcode;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		usage("Not enough parameters specified.");
	}

	if (strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"-?") == 0 || strcmp(argv[1],"/?") == 0) {
		usage();
	}

	try {
		for (int idx = 1; idx < argc; ++idx) {
			typePhysVersion(argv[idx]);
		}

	} catch (FileException &e) {
		std::cerr << "File exception occured trying to extract the chunk : " << e.getDetails() << std::endl;
	} catch (BasicException &e) {
		std::cerr << "Exception occured trying to extract the chunk : " << e.getDetails() << std::endl;
	}

	return 0;
}
