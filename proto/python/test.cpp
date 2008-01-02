// TEMPORARY CODE FOR FUNCTIONALITY TESTING, will be ERASED!
#include "test.h"
#include <iostream>
#include "bindings.h"

#include "logger.h"
#include "stdlog.h"
#include "OpdeServiceManager.h"
#include "ConfigService.h"
#include "LinkService.h"
#include "DatabaseService.h"
#include "GUIService.h"
#include "ScriptService.h"
#include "RenderService.h"
#include "PropertyService.h"
#include "BinaryService.h"
#include "InheritService.h"

#include "DTypeScriptLoader.h"
#include "PLDefScriptLoader.h"

#include "ConsoleBackend.h"

#include <OgreRoot.h>
#include <OgreConfigFile.h>


using namespace Opde;

void setupResources(void) {
	// First, register the script loaders...
	// Allocate and register the DType script loader. Registers itself
	DTypeScriptLoader* dtypeScriptLdr = new DTypeScriptLoader();  // !!!

	// Allocate and register the PLDef script loader. Registers itself
	PLDefScriptLoader* plDefScriptLdr = new PLDefScriptLoader();  // !!!

	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load("resources.cfg");

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;

    while (seci.hasMoreElements()) {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;

        for (i = settings->begin(); i != settings->end(); ++i) {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}


int main(void) {
	Logger* logger = new Logger();

	LogListener* stdLog = new StdLog();

	logger->registerLogListener(stdLog);

	ServiceManager* serviceMgr = new ServiceManager();
	PythonLanguage::init();

	// New service factory for Config service
	new ConfigServiceFactory();
	new LinkServiceFactory();
	new DatabaseServiceFactory();
	new RenderServiceFactory();
	new GUIServiceFactory();
	new ScriptServiceFactory();
	new PropertyServiceFactory();
	new BinaryServiceFactory();
	new InheritServiceFactory();

	new ConsoleBackend(); // !!!

    Ogre::Root* root = new Ogre::Root();

	// Run a python string to test the setup
	ConfigServicePtr configService = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();


    RenderServicePtr rendS = ServiceManager::getSingleton().getService("RenderService").as<RenderService>();

    setupResources();
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	ServiceManager::getSingleton().bootstrapFinished();

	// load a config file
	configService->setParam("test", "Hello World!");

	// To tidy mem...
	configService.setNull();

	LinkServicePtr ls = ServiceManager::getSingleton().getService("LinkService").as<LinkService>();

	// a simple int dtype, with default value 1
	DVariant zint = Opde::uint(1);
    DTypeDefPtr dint = new DTypeDef("int", zint, 4);

	RelationPtr r = ls->createRelation("TestRelation", dint, false);
	// Can't do mapping test without a data file - the mapping is determined from RELATIONS chunk

	r->setID(1);

	dint.setNull();
	r.setNull();
	ls.setNull();

    ScriptServicePtr ss = ServiceManager::getSingleton().getService("ScriptService").as<ScriptService>();

    ss->runScript("test.py");

    ss.setNull();

	PythonLanguage::term();


	delete root;

	delete serviceMgr;
	delete logger;

}
