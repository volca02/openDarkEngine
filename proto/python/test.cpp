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

#include "ServiceCommon.h"

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


void initLoopModes(LoopServicePtr& ls) {
	// Create all the required loop services
	LoopModeDefinition def;
	
	def.id = 1;
	def.name = "GUIOnlyLoopMode";
	def.mask = LOOPMODE_INPUT | LOOPMODE_RENDER;
	
	ls->createLoopMode(def);
	
	def.id = 0xFF;
	def.name = "AllClientsLoopMode";
	def.mask = LOOPMODE_MASK_ALL_CLIENTS;
	
	ls->createLoopMode(def);
}


int main(void) {
	Logger* logger = new Logger();

	LogListener* stdLog = new StdLog();

	logger->registerLogListener(stdLog);

	ServiceManager* serviceMgr = new ServiceManager();

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
	new LoopServiceFactory();

	new ConsoleBackend(); // !!!

    Ogre::Root* root = new Ogre::Root();

	// Run a python string to test the setup
	ConfigServicePtr configService = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();
	// Will load opde.cfg automatically upon being built
	// To tidy mem...
	configService.setNull();
	
    RenderServicePtr rendS = ServiceManager::getSingleton().getService("RenderService").as<RenderService>();
    rendS.setNull();

	// Resource initialization phase
    setupResources();
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	ServiceManager::getSingleton().bootstrapFinished();

    ScriptServicePtr ss = ServiceManager::getSingleton().getService("ScriptService").as<ScriptService>();

	LoopServicePtr ls = ServiceManager::getSingleton().getService("LoopService").as<LoopService>();

	initLoopModes(ls);

	ls.setNull();

	ss->runScript("test.py");
    ss.setNull();

	delete serviceMgr;
	
	delete root;
	delete logger;
}
