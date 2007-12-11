// TEMPORARY CODE FOR FUNCTIONALITY TESTING, will be ERASED!
#include "test.h"
#include <iostream>
#include "bindings.h"

#include "logger.h"
#include "stdlog.h"
#include "OpdeServiceManager.h"
#include "ConfigService.h"

using namespace Opde;


int main(void) {
	Logger* logger = new Logger();
	
	LogListener* stdLog = new StdLog();
	
	logger->registerLogListener(stdLog);
	
	ServiceManager* serviceMgr = new ServiceManager();
	PythonLanguage::init();

	// New service factory for Config service
	new ConfigServiceFactory();

	// Run a python string to test the setup
	ConfigServicePtr configService = ServiceManager::getSingleton().getService("ConfigService").as<ConfigService>();
	
	ServiceManager::getSingleton().bootstrapFinished();
	
	// load a config file
	configService->setParam("test", "Hello World!");
	
	// To tidy mem...
	configService.setNull();
	
	PyRun_SimpleString("from Opde import *\n"
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
	);
	
	PythonLanguage::term();
	
	delete serviceMgr;
	delete logger;
	
}
