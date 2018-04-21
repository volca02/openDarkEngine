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

#include "Relation.h"
#include "OpdeServiceManager.h"
#include "LinkCommon.h"
#include "inherit/InheritService.h"
#include "integers.h"
#include "logger.h"
#include "inherit/InheritService.h"

#include <stack>

using namespace std;

namespace Opde {
// size of the link header (id, src, dest, flav)
static size_t LinkStructSize = 14;

/*-----------------------------------------------------*/
/*--------------------- LinkQueries -------------------*/
/*-----------------------------------------------------*/
/// Single source link query (multiple targets), or in reverse
class Relation::MultiTargetLinkQueryResult : public LinkQueryResult {
public:
    MultiTargetLinkQueryResult(const Relation::ObjectIDToLinks &linkmap,
                               Relation::ObjectIDToLinks::const_iterator begin,
                               Relation::ObjectIDToLinks::const_iterator end)
        :

          LinkQueryResult(), mLinkMap(linkmap), mBegin(begin), mEnd(end) {
        mIter = mBegin;
    }

    virtual const LinkPtr &next() {
        assert(!end());

        const LinkPtr &l = mIter->second;

        ++mIter;

        return l;
    }

    virtual bool end() const { return (mIter == mEnd); }

protected:
    const Relation::ObjectIDToLinks &mLinkMap;
    Relation::ObjectIDToLinks::const_iterator mIter, mBegin, mEnd;
};

/// Link query that walks all ancestral objects as well
class Relation::InheritedMultiTargetLinkQueryResult : public LinkQueryResult {
public:
    InheritedMultiTargetLinkQueryResult(int objID, int dstID,
                                        const Relation *rel)
        : mOwner(rel), mSrcID(objID), mDstID(dstID) {
        // we need inherit service so we can ask for all object's sources
        mInheritService = GET_SERVICE(InheritService);
        mAncestorStack.push(mSrcID);
    }

    ~InheritedMultiTargetLinkQueryResult() { mInheritService.reset(); }

    virtual const LinkPtr &next() {
        // see if we have any more in the current iterator
        if (mCurrentIt || mCurrentIt->end()) {
            pollNextAncestor();
        }

        assert(!mCurrentIt.isNull());
        return mCurrentIt->next();
    }

    virtual bool end() const {
        if (!mCurrentIt)
            return mAncestorStack.empty();
        else
            return mCurrentIt->end() && mAncestorStack.empty();
    }

protected:
    void pollNextAncestor() {
        if (mAncestorStack.empty())
            return;

        int curId = mAncestorStack.top();

        // ask inherit service for list of ancestors
        InheritQueryResultPtr anci = mInheritService->getSources(curId);

        while (!anci->end()) {
            const InheritLinkPtr &l = anci->next();

            mAncestorStack.push(l->srcID);
        }

        // and unroll into the iterator
        mCurrentIt = mOwner->getAllLinks(curId, mDstID);
    }

    LinkQueryResultPtr mCurrentIt;
    std::stack<int> mAncestorStack;
    InheritServicePtr mInheritService;
    const Relation *mOwner;
    int mSrcID;
    int mDstID;
};

/*-----------------------------------------------------*/
/*----------------------- Relation --------------------*/
/*-----------------------------------------------------*/
Relation::Relation(const std::string &name, const DataStoragePtr &stor,
                   bool isInverse, bool hidden)
    : mSrcDstLinkMap(), mID(-1), mName(name), mStorage(stor), mHidden(hidden),
      mLinkMap(), mInverse(NULL), mIsInverse(isInverse) {

    // clear out the maximal ID info
    for (int i = 0; i < 16; ++i) {
        mMaxID[i] = 0;
    }

    if (!stor)
        mFakeSize = 0;
    else
        mFakeSize = mStorage->getDataSize();

    // Some default version values
    mLCVMaj = 2;
    mLCVMin = 0;
    mDCVMaj = 2;
    mDCVMin = 0;
}

// --------------------------------------------------------------------------
Relation::~Relation() {
    // deletion of Relation instance will not cause a messagging havok, as with
    // only deleting a single Link Only will inform about totally cleared DB
    clear();
}

// --------------------------------------------------------------------------
void Relation::load(const FileGroupPtr &db, const BitArray &objMask) {
    assert(!mIsInverse);

    // load the links and thei're data
    // Link chunk name
    string lchn = "L$" + mName;
    string ldchn = "LD$" + mName;

    FilePtr flink;
    FilePtr fldata;

    // let's open the L$NAME and LD$NAME files in the db
    try {
        flink = db->getFile(lchn);
    } catch (BasicException) {
        LOG_FATAL("Relation::load : Could not find the Link chunk %s",
                  lchn.c_str());
        return;
    }

    // Link data file:
    try {
        fldata = db->getFile(ldchn);
    } catch (BasicException &e) {
        if (!mStorage) {
            LOG_INFO("Relation::load : Link data chunk %s not found (It's ok "
                     "since data type not registered either)",
                     ldchn.c_str());
        } else {
            LOG_FATAL("Relation::load : Could not find the Link data chunk %s "
                      "with : %s",
                      ldchn.c_str(), e.getDetails().c_str());
            return;
        }
    }

    // link count (calculated)
    size_t link_count = flink->size() / LinkStructSize;

    LOG_VERBOSE("Relation::load : %s link count %d (tag size %d)", lchn.c_str(),
                link_count, flink->size());

    // if the chunk LD exists, and contains at least the data size, load the
    // data size, and set to load data as well
    bool load_data = false;
    size_t dsize = 0;

    // TODO: Defer this to link data dedicated class
    if (fldata->size() > sizeof(uint32_t)) {
        fldata->readElem(&dsize, sizeof(uint32_t));

        if (!mStorage)
            LOG_FATAL("Relation (%s): Data exist, but dyntype not set",
                      mName.c_str()); // Maybe I just should stop with exception
        else {
            load_data = true;

            // check for data len
            if (dsize != mStorage->getDataSize()) {
                // This just happens. Some links have the size totally different
                // the real

                // Only if we have the fake size wrong as well
                if (dsize != mFakeSize)
                    LOG_FATAL("Relation (%s): Data sizes differ : Type: %d, "
                              "Fake %d, Chunk: %d",
                              mName.c_str(), mStorage->getDataSize(), mFakeSize,
                              dsize);

                // we respect our data size
                dsize = mStorage->getDataSize();
            }

            // as the last thing, count the data entries
            // link_data_count = (fldata->size() - sizeof(uint32_t)) / dsize;
        }
    } else {
        if (mStorage)
            LOG_FATAL("Relation (%s): Link data not present in file, but type "
                      "defined",
                      mName.c_str());
    }

    // If data should be loaded, load the data chunk
    if (load_data) {
        assert(dsize > 0);

        // The count of data and links should be the same
        // assert(link_count == link_data_count);

        for (unsigned int idx = 0; idx < link_count; ++idx) {
            // Will get deleted automatically once the LinkPtr is released...
            // Link ID goes first, then the data
            link_id_t id;
            fldata->readElem(&id, 4);
            mStorage->readFromFile(fldata, id,
                                   false); // false - links don't store len

            LOG_VERBOSE("Relation (%s): Loaded link data for link id %d",
                        mName.c_str(), id);
        }
    }

    for (unsigned int idx = 0; idx < link_count; idx++) {
        LinkStruct slink;

        flink->readElem(&slink.id, sizeof(uint32_t));
        flink->readElem(&slink.src, sizeof(int32_t));
        flink->readElem(&slink.dest, sizeof(int32_t));
        flink->readElem(&slink.flavor, sizeof(uint16_t));

        LinkPtr link(new Link(slink));

        LOG_VERBOSE("Relation (%s - %d): Read link ID %d, from %d to %d "
                    "(F,C,IX: %d, %d, %d)",
                    mName.c_str(), mID, link->mID, link->mSrc, link->mDst,
                    LINK_ID_FLAVOR(link->mID), LINK_ID_CONCRETE(link->mID),
                    LINK_ID_INDEX(link->mID));

        // Check if the flavor fits
        // The mID can't be negative, but just for sure:
        assert(mID >= 0);
        assert(LINK_ID_FLAVOR(link->mID) ==
               static_cast<unsigned int>(mID)); // keep compiler happy

        // Look if we fit into the mask
        if (objMask[link->mSrc] && objMask[link->mDst]) {
            // Add link, notify listeners... Will search for data and throw if
            // did not find them
            _addLink(link);

            // Inverse relation will get an inverse link to use
            LinkPtr ilink = createInverseLink(link);

            mInverse->_addLink(ilink);
        } else {
            // the mask says no to the link!
            LOG_ERROR("Relation (%s - %d): Link (ID %d, %d to %d) thrown away "
                      "- obj IDs invalid",
                      mName.c_str(), mID, link->mID, link->mSrc, link->mDst);

            // delete the data as well...
            if (mStorage)
                mStorage->destroy(link->mID);
        }
    }

    LOG_DEBUG("Relation (%s - %d): Done!", mName.c_str(), mID);
}

// --------------------------------------------------------------------------
void Relation::save(const FileGroupPtr &db, uint saveMask) {
    assert(!mIsInverse);

    LOG_DEBUG("Relation::save Saving relation %s", mName.c_str());

    // Link chunk name
    string lchn = "L$" + mName;
    string ldchn = "LD$" + mName;

    FilePtr flnk = db->createFile(lchn, mLCVMaj, mLCVMin);
    FilePtr fldt = db->createFile(ldchn, mDCVMaj, mDCVMin);

    if (mStorage) {
        assert(mStorage->getDataSize() > 0);
    }

    // mmh. Rather always write the fake size. It seems to always be present
    fldt->writeElem(&mFakeSize, sizeof(uint32_t));

    // No need to order those. If it shows up that it would be actually better,
    // no problem sorting those by link_id_t just write the links as they go,
    // and write the data in parallel
    LinkMap::const_iterator it = mLinkMap.begin();

    // Write the links
    for (; it != mLinkMap.end(); ++it) {
        LinkPtr link = it->second;

        // Test against the link write mask
        int conc = LINK_ID_CONCRETE(link->mID);

        if (saveMask & (1 << conc)) { // mask says save!
            LinkStruct slink;

            slink.id = link->mID;
            slink.src = link->mSrc;
            slink.dest = link->mDst;
            slink.flavor = link->mFlavor;

            flnk->writeElem(&slink.id, sizeof(uint32_t));
            flnk->writeElem(&slink.src, sizeof(int32_t));
            flnk->writeElem(&slink.dest, sizeof(int32_t));
            flnk->writeElem(&slink.flavor, sizeof(uint16_t));
        } else {
            LOG_DEBUG("Relation (%s): Link concreteness of link %d was out of "
                      "requested : %d",
                      mName.c_str(), link->mID, conc);
        }
    }

    // if data are used, store
    if (mStorage) {
        IntIteratorPtr idit = mStorage->getAllStoredObjects();

        while (!idit->end()) {
            int32_t id = idit->next();

            // Test against the link write mask
            int conc = LINK_ID_CONCRETE(id);

            if (saveMask & (1 << conc)) { // mask says save!
                // TODO: What exactly is the rule that one should follow
                // selecting what to write into GAM/MIS? I mean: there is MP
                // link from 1 to some -X in GAM file. Hmmmm. (I guess this does
                // not matter for in-game)
                fldt->writeElem(&id, sizeof(link_id_t));

                if (!mStorage->writeToFile(fldt, id, false))
                    LOG_ERROR("There was an error writing link data %s for "
                              "object %d. Property was not loaded",
                              mName.c_str(), id);
            }
        }
    }
}

// --------------------------------------------------------------------------
Relation *Relation::inverse() {
    assert(mInverse != NULL);
    assert(mInverse->isInverse() == isInverse());

    return mInverse;
};

// --------------------------------------------------------------------------
void Relation::setInverseRelation(Relation *rel) {
    assert(mInverse == NULL);
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
    mSrcDstLinkMap.clear();

    if (mStorage)
        mStorage->clear();

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

    LinkPtr newl(new Link(id, from, to, mID));

    mStorage->create(id);

    // Last, insert the link to the database and notify
    _addLink(newl);

    LinkPtr ilink = createInverseLink(newl);

    mInverse->_addLink(ilink);

    return id;
}

// --------------------------------------------------------------------------
link_id_t Relation::createWithValues(int from, int to,
                                     const VariantStringMap &dataValues) {
    // Request an id. First let's see what concreteness we have
    unsigned int cidx = 0;

    if (from >= 0 || to >= 0)
        cidx = 1;

    link_id_t id = getFreeLinkID(cidx);

    LinkPtr newl(new Link(id, from, to, mID));

    mStorage->createWithValues(id, dataValues);

    // Last, insert the link to the database and notify
    _addLink(newl);

    LinkPtr ilink = createInverseLink(newl);

    mInverse->_addLink(ilink);

    return id;
}

// --------------------------------------------------------------------------
link_id_t Relation::createWithValue(int from, int to, const Variant &value) {
    // Request an id. First let's see what concreteness we have
    unsigned int cidx = 0;

    if (from >= 0 || to >= 0)
        cidx = 1;

    link_id_t id = getFreeLinkID(cidx);

    LinkPtr newl(new Link(id, from, to, mID));

    mStorage->createWithValue(id, value);

    // Last, insert the link to the database and notify
    _addLink(newl);

    LinkPtr ilink = createInverseLink(newl);

    mInverse->_addLink(ilink);

    return id;
}

// --------------------------------------------------------------------------
void Relation::remove(link_id_t id) {
    // A waste I smell here. Maybe there will be a difference in Broadcasts
    // later
    _removeLink(id);

    mInverse->_removeLink(id);
}

// --------------------------------------------------------------------------
bool Relation::setLinkField(link_id_t id, const std::string &field,
                            const Variant &value) {
    if (mStorage->setField(id, field, value)) {
        LinkChangeMsg m;

        m.change = LNK_CHANGED;
        m.linkID = id;

        // Inform the listeners about the change of data
        broadcastMessage(m);

        return true;
    } else {
        LOG_ERROR(
            "Relation::setLinkField : Link %d was not found in relation %d", id,
            mID);
        return false;
    }
}

// --------------------------------------------------------------------------
Variant Relation::getLinkField(link_id_t id, const std::string &field) {
    Variant value;

    if (mStorage->getField(id, field, value)) {
        return value;
    } else {
        LOG_ERROR(
            "Relation::getLinkField : Link %d was not found in relation %d", id,
            mID);
        return Variant();
    }
}

// --------------------------------------------------------------------------
LinkQueryResultPtr Relation::getAllLinks(int src, int dst) const {
    // based on case of the query, return result
    assert(src != 0); // Source can't be zero

    if (dst == 0) { // all link destinations

        ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(src);

        if (r != mSrcDstLinkMap.end()) {
            LinkQueryResultPtr res(new MultiTargetLinkQueryResult(
                r->second, r->second.begin(), r->second.end()));
            return res;
        }

    } else { // both dst is nonzero (one link destination)
        ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(src);

        if (r != mSrcDstLinkMap.end()) {
            // all the components of the secondary map go into the set
            ObjectIDToLinks::const_iterator ri = r->second.find(dst);

            if (ri != r->second.end()) {
                LinkQueryResultPtr res(new MultiTargetLinkQueryResult(
                    r->second, ri, r->second.upper_bound(dst)));
                return res;
            }
        }
    }

    LinkQueryResultPtr r(new EmptyLinkQueryResult());
    return r;
}

// --------------------------------------------------------------------------
LinkQueryResultPtr Relation::getAllInherited(int src, int dst) const {
    assert(src != 0); // Source can't be zero

    return LinkQueryResultPtr(
        new InheritedMultiTargetLinkQueryResult(src, dst, this));
}

// --------------------------------------------------------------------------
LinkPtr Relation::getOneLink(int src, int dst) const {
    LinkQueryResultPtr res = getAllLinks(src, dst);

    if (res->end())
        return LinkPtr(NULL);

    LinkPtr l = res->next();

    // I also could just return the first even if there would be more than one,
    // but that could lead to programmers headaches
    if (!res->end())
        OPDE_EXCEPT("More than one link fulfilled the requirement",
                    "Relation::getOneLink");

    return l;
}

// --------------------------------------------------------------------------
LinkPtr Relation::getLink(link_id_t id) const {
    LinkMap::const_iterator r = mLinkMap.find(id);

    if (r != mLinkMap.end()) {
        LinkPtr l =
            r->second; // I've heard direct return will confuse the ref-counting
        return l;
    } else
        return LinkPtr(NULL);
}

// --------------------------------------------------------------------------
void Relation::objectDestroyed(int id) {
    _objectDestroyed(id);
    mInverse->_objectDestroyed(id);
}

// --------------------------------------------------------------------------
const DataFields &Relation::getFieldDesc(void) {
    if (mStorage) {
        return mStorage->getFieldDesc();
    } else {
        static const DataFields empty;
        return empty;
    }
}

// --------------------------------------------------------------------------
void Relation::_addLink(const LinkPtr &link) {
    // Insert, and detect the presence of such link already inserted (same ID)
    std::pair<LinkMap::iterator, bool> ires =
        mLinkMap.insert(make_pair(link->mID, link));

    if (!ires.second) {
        LOG_ERROR("Relation: Found link with conflicting ID in relation %d "
                  "(%s): ID: %d (stored %d) - link already existed",
                  mID, mName.c_str(), link->mID, ires.first->second->mID);
    } else {
        // Verify link data exist
        if (mStorage && !mStorage->has(link->mID))
            OPDE_EXCEPT("Relation (" + mName +
                            "): Link Data not defined prior to link insertion",
                        "Relation::_addLink"); // for link id " + link->mID

        // Update the free link info
        allocateLinkID(link->mID);

        // Update the query databases
        // Src->Dst->LinkList
        pair<ObjectLinkMap::iterator, bool> r =
            mSrcDstLinkMap.insert(make_pair(link->mSrc, ObjectIDToLinks()));
        r.first->second.insert(make_pair(link->mDst, link));

        // fire the notification about inserted link
        LinkPtr lcopy(new Link(*link));
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

        // cycle through the result, find the occurence of the link with the
        // given ID, then remove
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

        mStorage->destroy(id);
    } else {
        LOG_ERROR(
            "Relation %d: Link requested for removal was not found :ID: %d",
            mID, id);
    }
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

    // I could make this customizable via some constructor parameter (having
    // index array and bool, iterating)
    if (mMaxID[cidx] == lidx) {
        mMaxID[cidx]--;
    }

    // TODO: insert the link id into available for ID reuse...
}

// --------------------------------------------------------------------------
LinkPtr Relation::createInverseLink(const LinkPtr &src) {
    LinkPtr inv(new Link(src->id(), src->dst(), src->src(), src->flavor()));

    return inv;
}

// --------------------------------------------------------------------------
void Relation::_objectDestroyed(int id) {
    assert(id != 0); // has to be nonzero. Zero is a wildcard

    ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(id);

    if (r != mSrcDstLinkMap.end()) {
        // I could just remove it, but let's be fair and broadcast
        // This will be very stormy. Maybe we will have to
        LinkQueryResultPtr res(new MultiTargetLinkQueryResult(
            r->second, r->second.begin(), r->second.end()));

        // We have the source object. Now branch on the dest
        while (!res->end()) {
            const LinkPtr &l = res->next();

            _removeLink(l->id());
            mInverse->_removeLink(l->id());
        }
    }
}
} // namespace Opde
