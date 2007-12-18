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

using namespace Opde;


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

	// Run a python string to test the setup
	ConfigServicePtr configService = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();
	
	ServiceManager::getSingleton().bootstrapFinished();
	
	// load a config file
	configService->setParam("test", "Hello World!");
	
	// To tidy mem...
	configService.setNull();
	
	LinkServicePtr ls = ServiceManager::getSingleton().getService("LinkService").as<LinkService>();
	
	RelationPtr r = ls->createRelation("TestRelation", NULL, false); 
	// Can't do mapping test without a data file - the mapping is determined from RELATIONS chunk
	
	r->setID(1);
	
	r.setNull();
	ls.setNull();
	
	PyRun_SimpleString(
		"from Opde import *\n"
		"print \"Loaded Opde module! Yay!\"\n"
		"s = Services.ConfigService()\n"
		"print \"Got config service! Yay Yay!\"\n"
		"print \"Do we have a parameter test?: '\" + str(s.hasParam(\"test\")) + \"'\";\n"
		"print \"Python read config parameter: '\" + s.getParam(\"test\") + \"'\";\n"
		"log_fatal(\"A fatal log test!\")\n"
		"log_error(\"Error test\")\n"
		"log_info(\"Info test\")\n"
		"log_debug(\"Debug test\")\n"
		"log_verbose(\"Verbose test\")\n"
		"ls = Services.LinkService()\n"
		"print \"Link flavor 0 name:\" + ls.flavorToName(0)\n" // Translate the link ID 1 to name
		"rel = ls.getRelation(\"TestRelation\")\n"
		"print \"Relation's flavor : \" + str(rel.getID())\n"
		"print \"Relation's name   : \" + rel.getName()\n"
	);
	
	PythonLanguage::term();
	
	delete serviceMgr;
	delete logger;
}
