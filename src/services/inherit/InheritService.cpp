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

#include "ServiceCommon.h"
#include "InheritService.h"
#include "logger.h"
#include "CachedInheritor.h"
#include "NeverInheritor.h"
#include "ArchetypeInheritor.h"

using namespace std;

namespace Opde {
	/*--------------------------------------------------------*/
	/*--------------------- InheritQueries -------------------*/
	/*--------------------------------------------------------*/
	/// Just an empty result of a query
    class EmptyInheritQueryResult : public InheritQueryResult {
	    public:
            EmptyInheritQueryResult() : InheritQueryResult() {  };

            virtual const InheritLinkPtr next() { return NULL; };

            virtual bool end() const {
                return true;
            };
	};

	class SimpleInheritQueryResult : public InheritQueryResult {
	    public:
            SimpleInheritQueryResult(const InheritService::InheritLinkMap& linkmap) : mLinkMap(linkmap), InheritQueryResult() {
                mIter = mLinkMap.begin();
            }

            virtual const InheritLinkPtr next() {
                if (!end()) {
                    InheritLinkPtr l = mIter->second;

                    ++mIter;

                    return l;
                } else {
                    return NULL;
                }
            }

            virtual bool end() const {
                return (mIter == mLinkMap.end());
            }

        protected:
            const InheritService::InheritLinkMap& mLinkMap;
            InheritService::InheritLinkMap::const_iterator mIter;
	};



	/*--------------------------------------------------------*/
	/*--------------------- InheritService -------------------*/
	/*--------------------------------------------------------*/
	InheritService::InheritService(ServiceManager *manager) : 
			Service(manager), 
			mMetaPropListenerID(0),	
			mMetaPropRelation() {
        // Register some common factories.
        // If a special factory would be needed, it has to be registered prior to it's usage, okay?
        InheritorFactoryPtr if_cached = new CachedInheritorFactory();
        addInheritorFactory(if_cached);

        InheritorFactoryPtr if_never = new NeverInheritorFactory();
        addInheritorFactory(if_never);

        InheritorFactoryPtr if_arch = new ArchetypeInheritorFactory();
        addInheritorFactory(if_arch);
	}

	//------------------------------------------------------
	InheritService::~InheritService() {
	    // It may so happen that the init() was not called...
	    if (!mMetaPropRelation.isNull())
            mMetaPropRelation->unregisterListener(mMetaPropListenerID);
	}

    //------------------------------------------------------
    void InheritService::addInheritorFactory(InheritorFactoryPtr factory) {
        mInheritorFactoryMap.insert(make_pair(factory->getName(), factory));
    }

    //------------------------------------------------------
    InheritorPtr InheritService::createInheritor(const std::string& name) {
        InheritorFactoryMap::iterator it = mInheritorFactoryMap.find(name);

        if (it != mInheritorFactoryMap.end()) {
            InheritorPtr inh = it->second->createInstance(this);
            mInheritors.push_back(inh); // no need map by name
            return inh;
        } else
            OPDE_EXCEPT(string("No inheritor factory found for name : ") + name, "InheritService::createInheritor");
    }

	//------------------------------------------------------
	void InheritService::init() {
   		// Link Service should have created us automatically through service masks.
		// So we can register as a link service listener
        LOG_INFO("InheritService::init()");

		Relation::ListenerPtr metaPropCallback = 
			new ClassCallback<LinkChangeMsg, InheritService>(this, &InheritService::onMetaPropMsg);

		// Get the LinkService, then the relation metaprop

        // contact the config. service, and look for the inheritance link name
		// TODO: ConfigurationService::getKey("Core","InheritanceLinkName").toString();

		mLinkService = ServiceManager::getSingleton().getService("LinkService").as<LinkService>();
		
		if (mLinkService.isNull())
		    OPDE_EXCEPT("LinkService does not exist?", "InheritService::init");

		mMetaPropRelation = mLinkService->getRelation("MetaProp");

		if (mMetaPropRelation.isNull())
		    OPDE_EXCEPT("MetaProp relation not found. Fatal.", "InheritService::init");

		mMetaPropListenerID = mMetaPropRelation->registerListener(metaPropCallback);

		LOG_DEBUG("InheritService::init() with this %X", this);

		LOG_INFO("InheritService::init() - done");
	}

	//------------------------------------------------------
	void InheritService::onMetaPropMsg(const LinkChangeMsg& msg) {
		LOG_DEBUG("InheritService::onMetaPropMsg() with this %X", this);

	    // If the message indicates reset of the database
	    if (msg.change == LNK_RELATION_CLEARED) {
	        clear(); // Will itself broadcast
	        return;
	    }

		// Read the link source, destination and priority

		// Get the priority of the link
        unsigned int priority;

        if (msg.change != LNK_REMOVED) // Do not waste time if the link is removed
            priority = mMetaPropRelation->getLinkField(msg.linkID, "priority").toUInt(); // Hardcoded! Could be parametrized

        // get the Link ref.
        LinkPtr l = mMetaPropRelation->getLink(msg.linkID);

        InheritChangeMsg smsg;
        smsg.srcID = l->dst(); // common
        smsg.dstID = l->src();

		if (msg.change == LNK_ADDED) {
		    _addLink(l, priority);
            // Insert into the maps
            smsg.change = INH_ADDED;

            broadcastMessage(smsg);
		} else if (msg.change == LNK_CHANGED) {
		    _changeLink(l, priority);
		    // Priority change happened - update
            smsg.change = INH_CHANGED;
            broadcastMessage(smsg);
		} else if (msg.change == LNK_REMOVED) {
		    broadcastMessage(smsg); // Remove will broadcast prior to the removal
		    _removeLink(l);
		    // Remove the link
            smsg.change = INH_REMOVED;
		}
	}

    //------------------------------------------------------
    void InheritService::clear() {
        // Step two. Broadcast the inheritance change
        InheritChangeMsg smsg;
        smsg.change = INH_CLEARED_ALL;
        smsg.srcID = 0; // Some dummy values
        smsg.dstID = 0;
        broadcastMessage(smsg);

        // Now clear
        mInheritSources.clear();
        mInheritTargets.clear();
    }

    //------------------------------------------------------
    InheritQueryResultPtr InheritService::getSources(int objID) const {
        InheritMap::const_iterator it = mInheritSources.find(objID);

        if (it != mInheritSources.end()) {
            InheritQueryResultPtr res = new SimpleInheritQueryResult(it->second);
            return res;
        } else {
            InheritQueryResultPtr res = new EmptyInheritQueryResult();
            return res;
        }

    }

    //------------------------------------------------------
	InheritQueryResultPtr InheritService::getTargets(int objID) const {
	    InheritMap::const_iterator it = mInheritTargets.find(objID);

        if (it != mInheritTargets.end()) {
            InheritQueryResultPtr res = new SimpleInheritQueryResult(it->second);
            return res;
        } else {
            InheritQueryResultPtr res = new EmptyInheritQueryResult();
            return res;
        }
	}

    //------------------------------------------------------
    void InheritService::_addLink(LinkPtr link, unsigned int priority) {
        // It works like this. link.src() is the target for inheritance, link.dst() is the source for inheritance
        InheritLinkPtr ilp = new InheritLink;

        // we've got the link reverse. MetaProp has src the target for inh, and dst the parent.
        ilp->srcID = link->dst();
        ilp->dstID = link->src();
        ilp->priority = priority;

        pair<InheritMap::iterator, bool> r = mInheritSources.insert(make_pair(ilp->dstID, InheritLinkMap()));

        // now insert for the link.src, with the InheritLinkPtr struct
        pair<InheritLinkMap::iterator, bool> ri = r.first->second.insert(make_pair(ilp->srcID, ilp));

        if (!ri.second)
            OPDE_EXCEPT("Multiple inheritance for the same src/dst pair is not allowed!", "InheritService::_addLink");

        // Repeat for the mInheritTarget's

        r = mInheritTargets.insert(make_pair(ilp->srcID, InheritLinkMap()));

        ri = r.first->second.insert(make_pair(ilp->dstID, ilp));

        if (!ri.second)
            OPDE_EXCEPT("Multiple inheritance for the same src/dst pair is not allowed!", "InheritService::_addLink");
    }

    //------------------------------------------------------
    void InheritService::_changeLink(LinkPtr link, unsigned int priority) {
        // Modify priority of the link
        InheritMap::iterator it = mInheritSources.find(link->dst());

        if (it != mInheritSources.end()) {
            // now insert for the link.src, with the InheritLinkPtr struct
            InheritLinkMap::iterator it2 = it->second.find(link->src());

            it2->second->priority = priority;
        } else
            OPDE_EXCEPT("Could not find the link to change the priority for", "InheritService::_changeLink");
    }

    //------------------------------------------------------
    void InheritService::_removeLink(LinkPtr link) {
// Modify priority of the link
        InheritMap::iterator it = mInheritSources.find(link->dst());

        if (it != mInheritSources.end()) {
            // now insert for the link.src, with the InheritLinkPtr struct
            InheritLinkMap::iterator it2 = it->second.find(link->src());

            if (it2 != it->second.end())
                it->second.erase(it2);
            else
                OPDE_EXCEPT("Could not find the link to change the priority for", "InheritService::_changeLink");
        } else
            OPDE_EXCEPT("Could not find the link to change the priority for", "InheritService::_changeLink");

        // Same again, for the targets
        it = mInheritTargets.find(link->src());
        if (it != mInheritTargets.end()) {
            // now insert for the link.src, with the InheritLinkPtr struct
            InheritLinkMap::iterator it2 = it->second.find(link->dst());

            if (it2 != it->second.end())
                it->second.erase(it2);
            else
                OPDE_EXCEPT("Could not find the link to change the priority for", "InheritService::_changeLink");
        } else
            OPDE_EXCEPT("Could not find the link to change the priority for", "InheritService::_changeLink");
    }

	//-------------------------- Factory implementation
	std::string InheritServiceFactory::mName = "InheritService";

	InheritServiceFactory::InheritServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& InheritServiceFactory::getName() {
		return mName;
	}

	const uint InheritServiceFactory::getMask() {
		return SERVICE_LINK_LISTENER;
	}

	Service* InheritServiceFactory::createInstance(ServiceManager* manager) {
		return new InheritService(manager);
	}

}
