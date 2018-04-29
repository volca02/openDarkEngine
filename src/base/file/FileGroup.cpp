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
 *	  $Id$
 *
 *****************************************************************************/

#include "FileGroup.h"
#include "format.h"

namespace Opde {
/*----------------------------------------------------*/
/*-------------------- FileGroup ---------------------*/
/*----------------------------------------------------*/
FileGroup::FileGroup(FilePtr &source) : mSrcFile(source) {}

//------------------------------------
FileGroup::FileGroup() : mSrcFile(NULL){};

//------------------------------------
FileGroup::~FileGroup() {}

std::string FileGroup::getName() {
    if (mSrcFile)
        return mSrcFile->getName();
    else
        return "";
}

/*--------------------------------------------------------*/
/*-------------------- DarkFileGroup ---------------------*/
/*--------------------------------------------------------*/
DarkFileGroup::DarkFileGroup(FilePtr &source) : FileGroup(source), mFiles() {
    if (mSrcFile) {
        // Read the header and stuff from the source file
        _initSource();
    }
}

//------------------------------------
DarkFileGroup::DarkFileGroup() : FileGroup() {}

//------------------------------------
DarkFileGroup::~DarkFileGroup() {
    // destroy all files in the map
    mFiles.clear();
}

//------------------------------------
void DarkFileGroup::_initSource() {
    assert(!mSrcFile.isNull());

    mSrcFile->seek(0);

    // read the header
    mSrcFile->readElem(&mHeader.inv_offset, 4);
    mSrcFile->readElem(&mHeader.zero, 4);
    mSrcFile->readElem(&mHeader.one, 4);
    mSrcFile->read(&mHeader.zeros, 256);
    mSrcFile->readElem(&mHeader.dead_beef, 4);

    // Sanity check
    if (mHeader.dead_beef != 0x0EFBEADDE) // Little endian encoded. would be
                                          // 0xDEADBEEF on big-endian, etc.
        OPDE_FILEEXCEPT(
            FILE_OTHER_ERROR,
            "Supplied file is not a Dark database file. Dead beef mismatch",
            "DarkFileGroup::_initSource()");

    // The inventory is the next thing to load
    mSrcFile->seek(mHeader.inv_offset);

    uint32_t chunkCount;

    // The count of chunks
    mSrcFile->readElem(&chunkCount, sizeof(uint32_t), 1);

    std::vector<DarkDBInvItem> inventory;
    inventory.resize(chunkCount);
    for (auto &inv : inventory) *mSrcFile >> inv;

    // Init a file for all chunks given
    for (const auto &item : inventory) {
        Chunk ch;

        mSrcFile->seek(item.offset);

        *mSrcFile >> ch.header;

        if (strncmp(ch.header.name, item.name, 12) != 0)
            OPDE_EXCEPT(format("Inventory chunk name mismatch: ",
                               ch.header.name, "-", item.name));

        ch.file = FilePtr(new FilePart(
            item.name, File::FILE_R, mSrcFile,
            item.offset + sizeof(ch.header), item.length));

        mFiles.insert(make_pair(std::string(item.name), ch));
    }
}

//------------------------------------
bool DarkFileGroup::hasFile(const std::string &name) const {
    ChunkMap::const_iterator it = mFiles.find(name);

    return (it != mFiles.end());
}

//------------------------------------
const DarkDBChunkHeader &DarkFileGroup::getFileHeader(const std::string &name) {
    ChunkMap::iterator it = mFiles.find(name);

    if (it != mFiles.end())
        return it->second.header;
    else
        OPDE_FILEEXCEPT(
            FILE_OP_FAILED,
            format("File named ", name, " was not found in this FileGroup"),
            "DarkFileGroup::getFile");
}

//------------------------------------
FilePtr DarkFileGroup::getFile(const std::string &name) {
    std::string sname;
    sname = name.substr(0, 11);

    ChunkMap::iterator it = mFiles.find(sname);

    if (it != mFiles.end()) {
        it->second.file->seek(0);
        return it->second.file;
    } else
        OPDE_FILEEXCEPT(
            FILE_OP_FAILED,
            format("File named ", sname, " was not found in this FileGroup"),
            "DarkFileGroup::getFile");
}

//------------------------------------
FilePtr DarkFileGroup::createFile(const std::string &name, uint32_t ver_maj,
                                  uint32_t ver_min) {
    // look if the file map contained a file of that name before
    ChunkMap::iterator it = mFiles.find(name);

    if (it != mFiles.end()) {
        OPDE_FILEEXCEPT(FILE_OTHER_ERROR,
                        format("Chunk already exists : " + name),
                        "DarkFileGroup::createFile");
    }

    Chunk ch;

    ch.header.version_high = ver_maj;
    ch.header.version_low = ver_min;
    ch.header.zero = 0;

    // zero out the chunk name
    memset(ch.header.name, 0, 12);
    name.copy(ch.header.name, 11);

    MemoryFile *newfile = new MemoryFile(name, File::FILE_RW);

    ch.file = FilePtr(newfile);

    mFiles.insert(make_pair(name, ch));

    return ch.file;
}

//------------------------------------
void DarkFileGroup::deleteFile(const std::string &name) {
    ChunkMap::iterator it = mFiles.find(name);

    if (it != mFiles.end()) {
        mFiles.erase(it);
    } else {
        OPDE_EXCEPT(format("File requested for deletion was not found : ", name));
    }
}

//------------------------------------
void DarkFileGroup::write(FilePtr &dest) {
    /*
      First, calculate the inventory offset, it is:
      Size of the main header. + size of all chunks (including the chunk header)
    */

    uint32_t inv_offset = sizeof(DarkDBHeader);

    ChunkMap::const_iterator it = mFiles.begin();

    uint32_t chunkcount = 0;

    for (; it != mFiles.end(); it++, chunkcount++) {
        inv_offset += sizeof(DarkDBChunkHeader) + it->second.file->size();
    }

    // Write the header
    mHeader.inv_offset = inv_offset;
    mHeader.zero = 0;
    mHeader.one = 1;

    memset(&mHeader.zeros, 0, sizeof(mHeader.zeros));

    mHeader.dead_beef = 0x0EFBEADDE;

    // --- Let's write the header
    mSrcFile->writeElem(&mHeader.inv_offset, 4);
    mSrcFile->writeElem(&mHeader.zero, 4);
    mSrcFile->writeElem(&mHeader.one, 4);
    mSrcFile->write(&mHeader.zeros, 256);
    mSrcFile->writeElem(&mHeader.dead_beef, 4);

    it = mFiles.begin();

    std::vector<DarkDBInvItem> inventory;
    inventory.resize(chunkcount);

    // Write the chunks
    for (int pos = 0; it != mFiles.end(); it++, pos++) {
        file_pos_t cfpos = dest->tell();

        // write the chunk header
        *dest << it->second.header;

        // write the chunk data
        it->second.file->writeToFile(*dest);

        strncpy(inventory[pos].name, it->second.header.name, 12);
        inventory[pos].offset = cfpos;
        inventory[pos].length = it->second.file->size();
    }

    // write the inventory size
    dest->writeElem(&chunkcount, sizeof(uint32_t));

    // write inventory
    for (auto &inv : inventory) *dest << inv;
}

//------------------------------------
FileGroup::const_iterator DarkFileGroup::begin() const {
    return mFiles.begin();
}

//------------------------------------
FileGroup::const_iterator DarkFileGroup::end() const { return mFiles.end(); }

} // namespace Opde
