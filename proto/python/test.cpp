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
#include "InputService.h"

#include "DTypeScriptLoader.h"
#include "PLDefScriptLoader.h"

#include "ConsoleBackend.h"

#include "ServiceCommon.h"

#ifdef CUSTOM_IMAGE_HOOKS
#include "CustomImageCodec.h"
#endif

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


void createServiceFactories() {
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
	new InputServiceFactory();

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
	LogListener* stdLog = new StdLog();
	Logger* logger = new Logger();
	logger->registerLogListener(stdLog);

	logger->log(Logger::LOG_INFO, "----- OPDE STARTING -----");

	Ogre::Root* root = new Ogre::Root();
	
#ifdef CUSTOM_IMAGE_HOOKS
	Ogre::CustomImageCodec::startup();
#endif

	PythonLanguage::init();

	ConsoleBackend* cbackend = new ConsoleBackend();

	/// Initializes all service factories and service manager itself
	ServiceManager* serviceMgr = new ServiceManager();

	createServiceFactories();

	/// ------------------ INITIALIZATION ----------------------
	logger->log(Logger::LOG_INFO, "----- INITIALIZATION -----");
	LoopServicePtr ls = ServiceManager::getSingleton().getService("LoopService").as<LoopService>();

	initLoopModes(ls);
	
	
	// Resource initialization phase
	/// ------------------ RESOURCE SETUP ----------------------
	logger->log(Logger::LOG_INFO, "----- RESOURCE SETUP -----");

    setupResources(); // Set's up resource paths, etc.

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();


	logger->log(Logger::LOG_INFO, "----- RESOURCE BOOSTRAP FINISHED -----");
	
	ServiceManager::getSingleton().bootstrapFinished(); // Notifying services bootstrap phase is over...

	logger->log(Logger::LOG_INFO, "----- CORE INITIALIZED, RUNNING MAIN SCRIPT -----");


    ScriptServicePtr ss = ServiceManager::getSingleton().getService("ScriptService").as<ScriptService>();
	ss->runScript("test.py");
    ss.setNull();

	logger->log(Logger::LOG_INFO, "----- DEINITIALIZATION -----");

	PythonLanguage::term();

	delete serviceMgr;
	
	logger->log(Logger::LOG_INFO, "----- ALL SERVICES STOPPED -----");

	delete cbackend;
	
#ifdef CUSTOM_IMAGE_HOOKS
	Ogre::CustomImageCodec::shutdown();
#endif
	
	delete root;

	logger->log(Logger::LOG_INFO, "----- TERMINATED -----");
	delete logger;
	delete stdLog;
}
