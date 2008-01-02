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


#ifndef __SCRIPTSERVICE_H
#define __SCRIPTSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include <set>
#include <list>
#include <map>

namespace Opde {

    /// A single instance of object script (instanced on object ID)
    class ObjectScript {
        public:
            ObjectScript(int id);

            virtual ~ObjectScript();

            // TODO: receiveMessage... etc.

        protected:
            int mID;
    };

    typedef shared_ptr<ObjectScript> ObjectScriptPtr;

    /// Factory for object scripts, as reported from scripting language.
    /// All that scripting lang side should do is implement the getScriptNames and createScript
    /// The scripts will be stand-alone objects inherited from some base parent class, and their
    /// destructor will Py_DECREF the callable python object, effectively removing it from the python VM
    class ObjectScriptModule {
        public:
            ObjectScriptModule(std::string& name);

            /// Returns true if this module handles a certain script type named name
            virtual bool handlesScript(std::string& name);

            /// Creates a new object script for object id object_id
            virtual ObjectScriptPtr createScript(int object_id, const std::string& name);

            /// Returns a std::set of script types that are handled by this module
            virtual std::set<std::string> getScriptNames();

            /// Returns script module name. Should be the same as the filename
            virtual std::string getName();

        protected:
            // typedef std::map<int, ObjectScriptPtr> ObjectIDToScript;
    };

    typedef shared_ptr<ObjectScriptModule> ObjectScriptModulePtr;

	/** @brief Script service - class responsible for loading and saving scripts and their interaction with the services
	* @note There are two kinds of scripts (logic-wise). The scripts that really are executed, then the object scripts.
	* @note PythonLanguage::init is called in the init method, PythonLanguage::term is called in the shutdown method
	* The initialization of object script module is based on python's report of such modules, but the mapping 1:1 with filename
	* is needed for the system to be able to know which script file contains which modules (that means gen.py should contain "gen" object script module).
	*/
	class ScriptService : public Service {
		public:
			/** Initializes the Service */
			ScriptService(ServiceManager* manager, const std::string& name);

			/** Destructs the service instance, and unallocates the data, if any. */
			virtual ~ScriptService();

            /// Runs a script file named filename (has to be found in ogre's resources)
            void runScript(const std::string& filename);

            /// registers a new object script module. Will live registered until an event does unload of database data
            void addObjectScriptModule(ObjectScriptModulePtr module);

		protected:
            bool init();
            void bootstrapFinished();
            void shutdown();

            typedef std::list<ObjectScriptModulePtr> ScriptModuleList;
            typedef std::map<std::string, ObjectScriptModulePtr> ScriptNameToModule;

            ScriptModuleList mScriptModules;
            ScriptNameToModule mScriptNameMap;

            void mapModuleScriptName(ObjectScriptModulePtr mod, std::string name);

            // TODO: Listeners - database and property object messages (Have to be written first) - here, we again reveal the need to only modify concrete throughout the game run.
            // TODO: per object script hierarchy. At the point of the loading (through DB service), the prop and link data are valid, so iteration on
            // object scripts should be enough (until don't inherit is found).
            // TODO: mapper: ObjectScriptModulePtr getModuleForScriptName(std::string& name), mapModuleScriptName(ObjectScriptModulePtr module, std::string& name) etc.
            // Here's the chellenge: When adding a metaproperty, all the scripts the object holds itself should be removed and the new ones instantinated.
            // (ignoring the ones that should stay there because the scripts should not hold any data themselves anyway).
            // Thus we need a way to say which effective object ID the given script comes from.
            // Another note: How shall we work with duplicities? IMHO any object should only hold one instance of the given script type, all duplicities should be ignored.
            // Another question: Are scripts comming from archetype still mapped on every concrete object? This seems likely.
            // We thus have creator id and holder id (creator is the archetype, holder the concrete obj)
            // In which order do the scripts receive an event? Does it come bottom to top (archetype to concrete) or in reverse?
            // A small todo: Script data service could be exposed nicely to python. A small cooperation, using getters and setters could let us do things like data['something'] = 1, etc.
            // With automatic mapping of object ID, internally calling ScriptDataService::get(id, name)... etc.
	};

	/// Shared pointer to script service
	typedef shared_ptr<ScriptService> ScriptServicePtr;

	/// Factory for the Script service
	class ScriptServiceFactory : public ServiceFactory {
		public:
			ScriptServiceFactory();
			~ScriptServiceFactory() {};

			/** Creates a ScriptService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

		private:
			static std::string mName;
	};
}


#endif
