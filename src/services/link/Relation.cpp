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


#include "integers.h"
#include "BinaryService.h"
#include "Relation.h"
#include "DTypeDef.h"
#include "LinkCommon.h"

#include "logger.h"

using namespace std;

namespace Opde {

    /*-----------------------------------------------------*/
	/*--------------------- LinkQueries -------------------*/
	/*-----------------------------------------------------*/
    /// Single source link query (multiple targets), or in reverse
	class Relation::MultiTargetLinkQueryResult : public LinkQueryResult {
	    public:
            MultiTargetLinkQueryResult(const Relation::ObjectIDToLinks& linkmap,
                                        Relation::ObjectIDToLinks::const_iterator begin,
                                        Relation::ObjectIDToLinks::const_iterator end) :

                            mLinkMap(linkmap),
                            mBegin(begin),
                            mEnd(end),
                            LinkQueryResult() {
                mIter = mBegin;
            }

            virtual const LinkPtr& next() {
                assert(!end());

				const LinkPtr& l = mIter->second;

				++mIter;

				return l;
            }

            virtual bool end() const {
                return (mIter == mEnd);
            }

        protected:
            const Relation::ObjectIDToLinks& mLinkMap;
            Relation::ObjectIDToLinks::const_iterator mIter, mBegin, mEnd;
	};

	/*-----------------------------------------------------*/
	/*----------------------- Relation --------------------*/
	/*-----------------------------------------------------*/
	Relation::Relation(const std::string& name, const DTypeDefPtr& type, bool isInverse, bool hidden) :
			mID(-1),
			mName(name),
			mType(type),
			mHidden(hidden),
			mLinkMap(),
			mSrcDstLinkMap(),
			mLinkDataMap(),
			mInverse(NULL),
			mIsInverse(isInverse) {

		// clear out the maximal ID info
		for (int i = 0; i < 16; ++i) {
			mMaxID[i] = 0;
		}

		if (mType.isNull())
				mFakeSize = 0;
			else
				mFakeSize = mType->size();

		// Some default version values
		mLCVMaj = 2;
		mLCVMin = 0;
		mDCVMaj = 2;
		mDCVMin = 0;
	}

	// --------------------------------------------------------------------------
	Relation::~Relation() {
		// deletion of Relation instance will not cause a messagging havok, as with only deleting a single Link
		// Only will inform about totally cleared DB
		clear();
	}

	// --------------------------------------------------------------------------
	void Relation::load(const FileGroupPtr& db) {
		assert(!mIsInverse);
		
		// load the links and thei're data
		BinaryServicePtr bs = ServiceManager::getSingleton().getService("BinaryService").as<BinaryService>();

		// Link chunk name
		string lchn = "L$" + mName;
		string ldchn = "LD$" + mName;

		FilePtr flink = NULL;
		FilePtr fldata = NULL;

		// let's open the L$NAME and LD$NAME files in the db
		try {
			flink = db->getFile(lchn);
		} catch (BasicException& e) {
			LOG_FATAL("Relation::load : Could not find the Link chunk %s", lchn.c_str());
			return;
		}

		// Link data file:
		try {
			fldata = db->getFile(ldchn);
		} catch (BasicException& e) {
			if (mType.isNull()) {
				LOG_INFO("Relation::load : Link data chunk %s not found (It's ok since data type not registered either)", ldchn.c_str());
			} else {
				LOG_FATAL("Relation::load : Could not find the Link data chunk %s with : %s", ldchn.c_str(), e.getDetails().c_str());
				return;
			}
		}

		DTypeDefPtr linkstruct = bs->getType("links", "LINK");

		size_t lsize = linkstruct->size();

		// now load the data
		size_t link_count = flink->size() / lsize;
		size_t link_data_count = 0;

		// if the chunk LD exists, and contains at least the data size, load the data size, and set to load data as well
		bool load_data = false;
		size_t dsize = 0;

		if (fldata->size() > sizeof(uint32_t)) {
			fldata->readElem(&dsize, sizeof(uint32_t));

			if (mType.isNull())
				LOG_FATAL("Relation (%s): Data exist, but dyntype not set", mName.c_str()); // Maybe I just should stop with exception
			else {
				load_data = true;

				// check for data len
				if (dsize != mType->size()) {
					// This just happens. Some links have the size totally different the real
					
					// Only if we have the fake size wrong as well
					if (dsize != mFakeSize)
						LOG_FATAL("Relation (%s): Data sizes differ : Type: %d, Fake %d, Chunk: %d", mName.c_str(), mType->size(), mFakeSize, dsize);
					
					// we respect our data size
					dsize = mType->size();
				}

				// as the last thing, count the data entries
				link_data_count = (fldata->size() - sizeof(uint32_t)) / dsize;
			}
		} else {
			if (!mType.isNull())
				LOG_FATAL("Relation (%s): Link data not present in file, but type defined", mName.c_str());
		}

		// If data should be loaded, load the data chunk
		if (load_data) {
			assert(dsize > 0);

			// The count of data and links should be the same
			// assert(link_count == link_data_count);

			for (unsigned int idx = 0; idx < link_count; idx++) {
				// Will get deleted automatically once the LinkPtr is released...
				// link->mData = new char[dsize];

				// Link ID goes first, then the data
				link_id_t id;
				fldata->readElem(&id, 4);

				LinkDataPtr ldta = new LinkData(id, mType, fldata, dsize, mUseDataCache);

				LOG_VERBOSE("Relation (%s): Loaded link data for link id %d", mName.c_str(), id);
				
				// Link data are inserted silently
				_assignLinkData(id, ldta);
				
				// And to the inverse relation as well
				mInverse->_assignLinkData(id, ldta);
			}
		}

		char* dlink = new char[lsize];

		for (unsigned int idx = 0; idx < link_count; idx++) {
			flink->read(dlink, lsize);

			LinkPtr link = LinkPtr(new Link(
				linkstruct->get(dlink, "id").toUInt(),
				linkstruct->get(dlink, "src").toInt(),
				linkstruct->get(dlink, "dest").toInt(),
				linkstruct->get(dlink, "flavor").toUInt()
			));

			LOG_VERBOSE("Relation (%s - %d): Read link ID %d, from %d to %d (F,C,IX: %d, %d, %d)",
				  mName.c_str(),
				  mID,
				  link->mID,
				  link->mSrc,
				  link->mDst,
				  LINK_ID_FLAVOR(link->mID),
				  LINK_ID_CONCRETE(link->mID),
				  LINK_ID_INDEX(link->mID)
				 );

			// Check if the flavor fits
			assert(LINK_ID_FLAVOR(link->mID) == mID);

			// Add link, notify listeners... Will search for data and throw if did not find them
			_addLink(link);
			
			// Inverse relation will get an inverse link to use
			LinkPtr ilink = createInverseLink(link);
			
			mInverse->_addLink(ilink);
		}

		delete[] dlink;

		LOG_DEBUG("Relation (%s - %d): Done!",
				  mName.c_str(),
				  mID);
	}

	// --------------------------------------------------------------------------
	void Relation::save(const FileGroupPtr& db, uint saveMask) {
		assert(!mIsInverse);
		
		LOG_DEBUG("Relation::save Saving relation %s", mName.c_str());

		// load the links and thei're data
		BinaryServicePtr bs = ServiceManager::getSingleton().getService("BinaryService").as<BinaryService>();

		// Link chunk name
		string lchn = "L$" + mName;
		string ldchn = "LD$" + mName;

		FilePtr flnk = db->createFile(lchn, mLCVMaj, mLCVMin);
		FilePtr fldt = db->createFile(ldchn, mDCVMaj, mDCVMin);

		uint32_t dtsz = 0;

		if (!mType.isNull()) {
			dtsz = mType->size();
			assert(dtsz > 0);
		}

		// mmh. Rather always write the fake size. It seems to always be present
		fldt->writeElem(&mFakeSize, sizeof(uint32_t));

		// No need to order those. If it shows up that it would be actually better, no problem sorting those by link_id_t
		DTypeDefPtr linkstruct = bs->getType("links", "LINK");

		char* lnkdta = linkstruct->create();

		// just write the links as they go, and write the data in parallel
		LinkMap::const_iterator it = mLinkMap.begin();

		// Write the links
		for (; it != mLinkMap.end(); ++it) {
			LinkPtr link = it->second;

			// Test against the link write mask
			int conc = LINK_ID_CONCRETE(link->mID);

			if (saveMask & (1 << conc)) { // mask says save!
				// write according to the structure
				linkstruct->set(lnkdta, "id", link->mID);
				linkstruct->set(lnkdta, "src", link->mSrc);
				linkstruct->set(lnkdta, "dest", link->mDst);
				linkstruct->set(lnkdta, "flavor", link->mFlavor);

				flnk->write(lnkdta, linkstruct->size());
			} else {
				LOG_DEBUG("Relation (%s): Link concreteness of link %d was out of requested : %d", mName.c_str(), link->mID, conc);
			}
		}

		delete lnkdta;

		// just write the links as they go, and write the data in parallel
		LinkDataMap::const_iterator dit = mLinkDataMap.begin();

		// Write the link data if exists
		if (!mType.isNull()) { // write the data into the data chunk
			for (; dit != mLinkDataMap.end(); ++dit) {
				LinkDataPtr ldt = dit->second;

				// Test against the link write mask
				int conc = LINK_ID_CONCRETE(ldt->mID);

				if (saveMask & (1 << conc)) { // mask says save!
					// TODO: What exactly is the rule that one should follow selecting what to write into GAM/MIS?
					// I mean: there is MP link from 1 to some -X in GAM file. Hmmmm. (I guess this does not matter for in-game)

					fldt->writeElem(&ldt->mID, sizeof(link_id_t));

					ldt->serialize(fldt);
				}
			}
		}
	}

	// --------------------------------------------------------------------------
	Relation* Relation::inverse() { 
		assert(mInverse != NULL);
		assert(mInverse->isInverse() == isInverse());
		
		return mInverse; 
	};
	
	// --------------------------------------------------------------------------
	void Relation::setInverseRelation(Relation* rel) { 
		assert(mInverse==NULL); 
		assert(rel->isInverse() != isInverse());
		
		mInverse = rel; 
	};
	
	// --------------------------------------------------------------------------
	void Relation::clear() {
		// first, broadcast that we're gonna erase
		LinkChangeMsg m;

		m.change = LNK_RELATION_CLEARED;
		m.linkID = 0; // all links, no meaning

		// Inform the listeners about the change of data
		broadcastMessage(m);

		mLinkMap.clear();
		mLinkDataMap.clear();
		mSrcDstLinkMap.clear();


		// clear maximal link id's
		for (int i = 0; i < 16; ++i) {
			mMaxID[i] = 0;
		}

		// Done...
	}


	// --------------------------------------------------------------------------
	link_id_t Relation::create(int from, int to) {
		// Request an id. First let's see what concreteness we have
		unsigned int cidx = 0;

		if (from >= 0 || to >= 0)
			cidx = 1;

		link_id_t id = getFreeLinkID(cidx);

		LinkPtr newl = new Link(id, from, to, mID);

		LinkDataPtr newd = new LinkData(id, mType, mUseDataCache);

		// assign link data in advance, as it would fail on _addLink otherwise
		_assignLinkData(id, newd);

		mInverse->_assignLinkData(id, newd);

		// Last, insert the link to the database and notify
		_addLink(newl);
		
		LinkPtr ilink = createInverseLink(newl);
		
		mInverse->_addLink(ilink);

		return id;
	}

	// --------------------------------------------------------------------------
	link_id_t Relation::create(int from, int to, const DTypePtr& data) {
		// Request an id. First let's see what concreteness we have
		unsigned int cidx = 0;

		// simply compare the type pointers...
		if (data->type() != mType)
			OPDE_EXCEPT("Incompatible types when creating link data", "Relation::create");

		if (from >= 0 || to >= 0)
			cidx = 1;

		link_id_t id = getFreeLinkID(cidx);

		LinkPtr newl = new Link(id, from, to, mID);

		LinkDataPtr newd = new LinkData(id, data, mUseDataCache);

		// assign link data in advance, as it would fail on _addLink otherwise
		_assignLinkData(id, newd);
		
		mInverse->_assignLinkData(id, newd);

		// Last, insert the link to the database and notify
		_addLink(newl);
		
		LinkPtr ilink = createInverseLink(newl);
		
		mInverse->_addLink(ilink);


		return id;
	}

	// --------------------------------------------------------------------------
	void Relation::remove(link_id_t id) {
		// A waste I smell here. Maybe there will be a difference in Broadcasts later
		_removeLink(id);
		
		mInverse->_removeLink(id);
	}

	// --------------------------------------------------------------------------
	void Relation::setLinkField(link_id_t id, const std::string& field, const DVariant& value) {
		LinkDataMap::iterator it = mLinkDataMap.find(id);

		if (it != mLinkDataMap.end()) {
			mType->set(it->second->mData, field, value);

			LinkChangeMsg m;

			m.change = LNK_CHANGED;
			m.linkID = id;

			// Inform the listeners about the change of data
			broadcastMessage(m);
		} else {
			LOG_ERROR("Relation::setLinkField : Link %d was not found in relation %d", id, mID);
		}
	}

	// --------------------------------------------------------------------------
	DVariant Relation::getLinkField(link_id_t id, const std::string& field) {
		LinkDataMap::const_iterator it = mLinkDataMap.find(id);

		if (it != mLinkDataMap.end()) {
			return mType->get(it->second->mData, field);
		} else {
			LOG_ERROR("Relation::getLinkField : Link %d was not found in relation %d", id, mID);
			return DVariant();
		}
	}

	// --------------------------------------------------------------------------
	void Relation::setLinkData(link_id_t id, char* data) {
		LinkDataMap::iterator it = mLinkDataMap.find(id);

		if (it != mLinkDataMap.end()) {
			// swap the data
			char* odl = it->second->mData;
			it->second->mData = data;
			delete odl;

			LinkChangeMsg m;

			m.change = LNK_CHANGED;
			m.linkID = id;

			// Inform the listeners about the change of data
			broadcastMessage(m);
		} else {
			LOG_ERROR("Relation::setLinkData : Link data %d was not found in relation %d", id, mID);
		}
	}

	// --------------------------------------------------------------------------
	LinkDataPtr Relation::getLinkData(link_id_t id) {
		LinkDataMap::iterator it = mLinkDataMap.find(id);

		if (it != mLinkDataMap.end()) {
			return it->second;
		} else {
			LOG_ERROR("Relation::getLinkData : Link data %d was not found in relation %d", id, mID);
			return NULL;
		}
	}

	// --------------------------------------------------------------------------
	LinkQueryResultPtr Relation::getAllLinks(int src, int dst) const {
		// based on case of the query, return result
		assert(src != 0); // Source can't be zero

		if (dst == 0) { // all link destinations

			ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(src);

			if (r != mSrcDstLinkMap.end()) {
    			LinkQueryResultPtr res;

				// We have the source object. Now branch on the dest
			    res = new MultiTargetLinkQueryResult(r->second, r->second.begin(), r->second.end());

				return res;
			}

		} else { // both dst is nonzero (one link destination)
			ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(src);

			if (r != mSrcDstLinkMap.end()) {
				// all the components of the secondary map go into the set
				ObjectIDToLinks::const_iterator ri = r->second.find(dst);

				if (ri != r->second.end()) {
	    			LinkQueryResultPtr res;
                    res = new MultiTargetLinkQueryResult(r->second, ri, r->second.upper_bound(dst));
                    return res;
				}
			}
		}

        LinkQueryResultPtr r = new EmptyLinkQueryResult();
        return r;
	}

	// --------------------------------------------------------------------------
	LinkPtr Relation::getOneLink(int src, int dst) const {
		LinkQueryResultPtr res = getAllLinks(src, dst);

        if (res->end())
            return NULL;

        LinkPtr l = res->next();

		// I also could just return the first even if there would be more than one, but that could lead to programmers headaches
		if (!res->end())
            OPDE_EXCEPT("More than one link fulfilled the requirement", "Relation::getOneLink");

		return l;
	}

	// --------------------------------------------------------------------------
	LinkPtr Relation::getLink(link_id_t id) const {
		LinkMap::const_iterator r = mLinkMap.find(id);

		if (r != mLinkMap.end()) {
			LinkPtr l = r->second; // I've heard direct return will confuse the ref-counting
			return l;
		} else
			return NULL;
	}

	void Relation::objectDestroyed(int id) {
		_objectDestroyed(id);
		mInverse->_objectDestroyed(id);
	}

	// --------------------------------------------------------------------------
	void Relation::_addLink(const LinkPtr& link) {
		// Insert, and detect the presence of such link already inserted (same ID)
		std::pair<LinkMap::iterator, bool> ires = mLinkMap.insert(make_pair(link->mID, link));

		if (!ires.second) {
			LOG_ERROR("Relation: Found link with conflicting ID in relation %d (%s): ID: %d (stored %d) - link already existed", mID, mName.c_str(), link->mID, ires.first->second->mID );
		} else {
            // Verify link data exist
			LinkDataMap::iterator dit = mLinkDataMap.find(link->mID);

			if (dit == mLinkDataMap.end() && !mType.isNull())
				OPDE_EXCEPT("Relation (" + mName + "): Link Data not defined prior to link insertion", "Relation::_addLink"); // for link id " + link->mID

			// Update the free link info
			allocateLinkID(link->mID);

			// Update the query databases
			// Src->Dst->LinkList
			pair<ObjectLinkMap::iterator, bool> r = mSrcDstLinkMap.insert(make_pair(link->mSrc, ObjectIDToLinks()));
			r.first->second.insert(make_pair(link->mDst, link));

			// fire the notification about inserted link
			LinkPtr lcopy = new Link(*link);
			LinkChangeMsg m(lcopy);

			m.change = LNK_ADDED;
			m.linkID = link->mID;

			// Inform the listeners about the change
			broadcastMessage(m);
		}
	}

	// --------------------------------------------------------------------------
	void Relation::_removeLink(link_id_t id) {
		LinkMap::iterator it = mLinkMap.find(id);

		if (it != mLinkMap.end()) {
			unallocateLinkID(id);

			LinkPtr to_remove = it->second;

			// Update the query databases
			// SrcDst
			ObjectLinkMap::iterator r = mSrcDstLinkMap.find(to_remove->mSrc);

			assert(r != mSrcDstLinkMap.end());

			ObjectIDToLinks::iterator ri = r->second.find(to_remove->mDst);
			ObjectIDToLinks::iterator rend = r->second.upper_bound(to_remove->mDst);

			assert(ri != r->second.end());

            // cycle through the result, find the occurence of the link with the given ID, then remove
            for (; ri != rend; ++ri) {
                if (ri->second->mID == id) {
                    r->second.erase(ri);
                }
            }

			// DstSrc
			r = mSrcDstLinkMap.find(to_remove->mSrc);

			assert(r != mSrcDstLinkMap.end());

			ri = r->second.find(to_remove->mDst);
			rend = r->second.upper_bound(to_remove->mDst);

			assert(ri != r->second.end());

			for (; ri != rend; ++ri) {
                if (ri->second->mID == id) {
                    r->second.erase(ri);
                }
            }

			// fire the notification about inserted link
			LinkChangeMsg m;

			m.change = LNK_REMOVED;
			m.linkID = id;

			// Inform the listeners about the change
			broadcastMessage(m);

			// last, erase the link
			mLinkMap.erase(it);

			_removeLinkData(id);
		} else {
			LOG_ERROR("Relation %d: Link requested for removal was not found :ID: %d", mID, id);
		}
	}

	// --------------------------------------------------------------------------
	void Relation::_assignLinkData(link_id_t id, const LinkDataPtr& data) {
		std::pair<LinkDataMap::iterator, bool> ires = mLinkDataMap.insert(make_pair(id, data));

		if (!ires.second) {
			// data already present
			ires.first->second = data;
		}

		// Done.
	}

	// --------------------------------------------------------------------------
	void Relation::_removeLinkData(link_id_t id) {
		mLinkDataMap.erase(id);
	}

	// --------------------------------------------------------------------------
	link_id_t Relation::getFreeLinkID(uint cidx) {
		link_id_t id = LINK_MAKE_ID(mID, cidx, mMaxID[cidx] + 1);
		allocateLinkID(id);
		return id;
	}

	// --------------------------------------------------------------------------
	void Relation::allocateLinkID(link_id_t id) {
		unsigned int cidx = LINK_ID_CONCRETE(id);

		assert(cidx < 16);

		link_id_t lidx = LINK_ID_INDEX(id);

		if (mMaxID[cidx] < lidx) {
			mMaxID[cidx] = lidx;
		}
	}

	// --------------------------------------------------------------------------
	void Relation::unallocateLinkID(link_id_t id) {
		unsigned int cidx = LINK_ID_CONCRETE(id);

		assert(cidx < 16);

		link_id_t lidx = LINK_ID_INDEX(id);

		// I could make this customisible via some constructor parameter (having index array and bool, iterating)
		if (mMaxID[cidx] == lidx) {
			mMaxID[cidx]--;
		}
		
		// TODO: insert the link id into available for ID reuse...
	}
	
	// --------------------------------------------------------------------------
	LinkPtr Relation::createInverseLink(const LinkPtr& src) {
		LinkPtr inv = new Link(
			src->id(),
			src->dst(),
			src->src(),
			src->flavor()
		);
		
		return inv;
	}
	
	// --------------------------------------------------------------------------
	void Relation::_objectDestroyed(int id) {
		assert(id != 0); // has to be nonzero. Zero is a wildcard

		ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(id);

		if (r != mSrcDstLinkMap.end()) {
			// I could just remove it, but let's be fair and broadcast
			// This will be very stormy. Maybe we will have to 
			LinkQueryResultPtr res;

			// We have the source object. Now branch on the dest
			res = new MultiTargetLinkQueryResult(r->second, r->second.begin(), r->second.end());

			while (!res->end()) {
				const LinkPtr& l = res->next();
				
				_removeLink(l->id());
				mInverse->_removeLink(l->id());
			}
		}
	}
}
