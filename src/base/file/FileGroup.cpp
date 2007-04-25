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
 
#include "FileGroup.h"

using namespace std;

namespace Opde {
	/*----------------------------------------------------*/
	/*-------------------- FileGroup ---------------------*/
	/*----------------------------------------------------*/
	FileGroup::FileGroup(File* source) : mSrcFile(source) {
		mSrcFile->addRef();
	}
	
	//------------------------------------	
	FileGroup::FileGroup() : mSrcFile(NULL) {};
	
	//------------------------------------	
	FileGroup::~FileGroup() {
		if (mSrcFile != NULL)
			mSrcFile->release();
	}

	
	/*--------------------------------------------------------*/
	/*-------------------- DarkFileGroup ---------------------*/
	/*--------------------------------------------------------*/
	DarkFileGroup::DarkFileGroup(File* source) : FileGroup(source), mFiles() {
		
		if (mSrcFile != NULL) {
			// Read the header and stuff from the source file
			_initSource();
		}
	}
	
	//------------------------------------	
	DarkFileGroup::DarkFileGroup() : FileGroup() {
		
	}
	
	//------------------------------------	
	DarkFileGroup::~DarkFileGroup() {
		// destroy all files in the map
		ChunkMap::iterator it = mFiles.begin();
		
		for (; it!=mFiles.end(); it++) {
			it->second.file->release();
		}
		
		mFiles.clear();
	}
	
	//------------------------------------	
	void DarkFileGroup::_initSource() {
		assert(mSrcFile != NULL);
		
		mSrcFile->seek(0);
		
		// read the header. Field after field
		mSrcFile->readElem(&mHeader.inv_offset, sizeof(mHeader.inv_offset), 1);
		mSrcFile->readElem(&mHeader.zero, sizeof(mHeader.zero), 1);
		mSrcFile->readElem(&mHeader.one, sizeof(mHeader.one), 1);
		
		// just a couple of zero bytes
		mSrcFile->read(&mHeader.zeros, sizeof(mHeader.zeros));
		
		// dead_beef at end
		mSrcFile->readElem(&mHeader.dead_beef, sizeof(mHeader.dead_beef), 1);
		
		// -- Header read
		
		// Sanity check
		if (mHeader.dead_beef != 0x0EFBEADDE) // Little endian encoded. would be 0xDEADBEEF on big-endian, etc.
			OPDE_FILEEXCEPT(FILE_OTHER_ERROR, "Supplied file is not a Dark database file. Dead beef mismatch", "DarkFileGroup::_initSource()");
		
		// The inventory is the next thing to load
		mSrcFile->seek(mHeader.inv_offset);
		
		uint32_t chunkCount;
		
		// The count of chunks
		mSrcFile->readElem(&chunkCount, sizeof(uint32_t), 1);
		
		DarkDBInvItem* inventory = new DarkDBInvItem[chunkCount];
		
		readInventory(inventory, chunkCount);
		
		// Init a file for all chunks given
		for (uint idx = 0; idx < chunkCount; idx++) {
			Chunk ch;
			
			mSrcFile->seek(inventory[idx].offset);
			
			readChunkHeader(&ch.header);
			
			if (strncmp(ch.header.name, inventory[idx].name, 12) != 0)
				OPDE_EXCEPT(string("Inventory chunk name mismatch: ") + ch.header.name + "-" + inventory[idx].name,
					    "DarkFileGroup::_initSource");
			
			ch.file = new FilePart(inventory[idx].name, File::FILE_R, mSrcFile, inventory[idx].offset + sizeof(ch.header), inventory[idx].length);
			
			mFiles.insert(make_pair(std::string(inventory[idx].name), ch));
		}
		
		delete[] inventory;
	}
	
	//------------------------------------	
	void DarkFileGroup::readInventory(DarkDBInvItem* inventory, uint count) {
		assert(mSrcFile != NULL);
		
		for (uint i = 0; i < count; i++) {
			mSrcFile->read(&inventory[i].name, sizeof(inventory[i].name));
			mSrcFile->readElem(&inventory[i].offset, sizeof(inventory[i].offset), 1);
			mSrcFile->readElem(&inventory[i].length, sizeof(inventory[i].length), 1);
		}
	}
	
	//------------------------------------	
	void DarkFileGroup::writeInventory(File* dest, DarkDBInvItem* inventory, uint count) {
		assert(dest != NULL);
		
		for (uint i = 0; i < count; i++) {
			dest->write(&inventory[i].name, sizeof(inventory[i].name));
			dest->writeElem(&inventory[i].offset, sizeof(inventory[i].offset), 1);
			dest->writeElem(&inventory[i].length, sizeof(inventory[i].length), 1);
		}
	}

	
	//------------------------------------	
	void DarkFileGroup::readChunkHeader(DarkDBChunkHeader* hdr) {
		assert(mSrcFile != NULL);
		
		mSrcFile->read(&hdr->name, sizeof(hdr->name));
		mSrcFile->readElem(&hdr->version_high, sizeof(hdr->version_high), 1);
		mSrcFile->readElem(&hdr->version_low, sizeof(hdr->version_low), 1);
		mSrcFile->readElem(&hdr->zero, sizeof(hdr->zero), 1);
	}
	
	//------------------------------------	
	void DarkFileGroup::writeChunkHeader(File* dest, const DarkDBChunkHeader& hdr) {
		assert(dest != NULL);
		
		dest->write(&hdr.name, sizeof(hdr.name));
		dest->writeElem(&hdr.version_high, sizeof(hdr.version_high), 1);
		dest->writeElem(&hdr.version_low, sizeof(hdr.version_low), 1);
		dest->writeElem(&hdr.zero, sizeof(hdr.zero), 1);
	}
	
	//------------------------------------	
	bool DarkFileGroup::hasFile(const std::string& name) const {
		ChunkMap::const_iterator it = mFiles.find(name);
		
		return (it != mFiles.end());
	}
	
	const DarkDBChunkHeader& DarkFileGroup::getFileHeader(const std::string& name) {
		ChunkMap::iterator it = mFiles.find(name);
		
		if (it != mFiles.end())
			return it->second.header;
		else
			OPDE_FILEEXCEPT(FILE_OP_FAILED, string("File named ") + name + " was not found in this FileGroup", "DarkFileGroup::getFile");
	}
	
	//------------------------------------	
	File* DarkFileGroup::getFile(const std::string& name) {
		ChunkMap::iterator it = mFiles.find(name);
		
		if (it != mFiles.end()) {
			it->second.file->addRef();
			return it->second.file;
		} else
			OPDE_FILEEXCEPT(FILE_OP_FAILED, string("File named ") + name + " was not found in this FileGroup", "DarkFileGroup::getFile");
	}
	
	//------------------------------------	
	File* DarkFileGroup::createFile(const std::string& name, uint32_t ver_maj, uint32_t ver_min) {
		// look if the file map contained a file of that name before
		ChunkMap::iterator it = mFiles.find(name);
		
		if (it != mFiles.end()) {
			OPDE_FILEEXCEPT(FILE_OTHER_ERROR, string("Chunk already exists : ") + name, "DarkFileGroup::createFile");
		}
		
		Chunk ch;
		
		ch.header.version_high = ver_maj;
		ch.header.version_low = ver_min;
		ch.header.zero = 0;
		
		// zero out the chunk name
		memset(ch.header.name, 0, 12);
		name.copy(ch.header.name, 11);
				
		MemoryFile* newfile = new MemoryFile(name, File::FILE_RW);
		
		ch.file = newfile;
		
		mFiles.insert(make_pair(name, ch));
		
		newfile->addRef();
		return newfile;
	}
			
	//------------------------------------	
	void DarkFileGroup::deleteFile(const std::string& name) {
		ChunkMap::iterator it = mFiles.find(name);
		
		if (it != mFiles.end()) {
			it->second.file->release();
			
			mFiles.erase(it);
		} else {
			OPDE_EXCEPT(string("File requested for deletion was not found : ") + name, "DarkFileGroup::deleteFile");
		}
	}
	
	//------------------------------------	
	void DarkFileGroup::write(File* dest) {
		/*
		First, calculate the inventory offset, it is:
		Size of the main header. + size of all chunks (including the chunk header)
		*/
		
		uint32_t inv_offset = sizeof(DarkDBHeader);
		
		ChunkMap::const_iterator it = mFiles.begin();
		
		uint32_t chunkcount = 0;
		
		for (; it!=mFiles.end(); it++, chunkcount++) {
			inv_offset += sizeof(DarkDBChunkHeader) + it->second.file->size();
		}
		
		
		// Write the header
		mHeader.inv_offset = inv_offset;
		mHeader.zero = 0;
		mHeader.one = 1;
		
		memset(&mHeader.zeros, 0, sizeof(mHeader.zeros));
		
		mHeader.dead_beef = 0x0EFBEADDE;
		
		// --- Let's write the header
		
		// read the header. Field after field
		dest->writeElem(&mHeader.inv_offset, sizeof(mHeader.inv_offset), 1);
		dest->writeElem(&mHeader.zero, sizeof(mHeader.zero), 1);
		dest->writeElem(&mHeader.one, sizeof(mHeader.one), 1);
		
		// just a couple of zero bytes
		dest->write(&mHeader.zeros, sizeof(mHeader.zeros));
		
		dest->writeElem(&mHeader.dead_beef, sizeof(mHeader.dead_beef), 1);
		
		// --- End of the header write
		
		
		it = mFiles.begin();
		
		DarkDBInvItem* inventory = new DarkDBInvItem[chunkcount];
		
		// Write the chunks
		for (int pos = 0; it != mFiles.end(); it++, pos++) {
			file_pos_t cfpos = dest->tell();
			
			// write the chunk header
			writeChunkHeader(dest, it->second.header);
			
			// write the chunk data
			it->second.file->writeToFile(*dest);
			
			strncpy(inventory[pos].name, it->second.header.name, 12);
			inventory[pos].offset = cfpos;
			inventory[pos].length = it->second.file->size();
		}
		
		// write the inventory
		dest->writeElem(&chunkcount, sizeof(uint32_t));
		writeInventory(dest, inventory, chunkcount);
		
		// get rid of temporary
		delete[] inventory;
	}
	
	//------------------------------------	
	FileGroup::const_iterator DarkFileGroup::begin() const {
		return mFiles.begin();
	}
	
	//------------------------------------	
	FileGroup::const_iterator DarkFileGroup::end() const {
		return mFiles.end();
	}
	
}
