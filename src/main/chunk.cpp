/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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

using namespace Opde;

void usage(const char *message = NULL) {
    if (message)
        std::cerr << message << std::endl;

    std::cout << "chunk FILE [CHUNK OUTFILE]" << std::endl
              << "  FILE - the dark database file to read" << std::endl
              << "  CHUNK - the chunk name to extract" << std::endl
              << "  OUTFILE - the file that the chunk is written into"
              << std::endl
              << "  with just the first argument, the program lists the chunks "
                 "contained: CHUNK_NAME SIZE VERSION"
              << std::endl;

    exit(1);
}

void listChunks(FileGroup &gr) {
    for (const auto &c : gr) {
        const DarkDBChunkHeader &head = c.second.header;

        printf("%-12s %8ld %4d.%d\n", c.first.c_str(),
               c.second.file->size(), head.version_high, head.version_low);
    }
}

int main(int argc, char *argv[]) {
    bool display = false; // only list the chunks

    if (argc < 2) {
        usage("Not enough parameters specified.");
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-?") == 0 ||
        strcmp(argv[1], "/?") == 0) {
        usage();
    }

    if (argc == 2) {
        display = true;
    } else if (argc == 3) {
        usage("Output file not specified or invalid commandline parameters");
    }

    try {
        FilePtr src = FilePtr(new StdFile(argv[1], File::FILE_R));
        DarkFileGroup gr (src);

        if (!display) {
            FilePtr chunk = gr.getFile(argv[2]);

            StdFile dest(argv[3], File::FILE_W);

            chunk->writeToFile(dest);
        } else {
            listChunks(gr);
        }
    } catch (FileException &e) {
        std::cerr << "File exception occured trying to extract the chunk : "
                  << e.getDetails() << std::endl;
    } catch (BasicException &e) {
        std::cerr << "Exception occured trying to extract the chunk : "
                  << e.getDetails() << std::endl;
    }

    return 0;
}
