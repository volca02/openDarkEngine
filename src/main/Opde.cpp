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
 *    $Id$
 *
 *****************************************************************************/

#include <iostream>

#include "Root.h"
#include "OpdeException.h"
#include "StringTokenizer.h"
#include "ServiceCommon.h"

#include "config/ConfigService.h"
#include "loop/LoopService.h"
#include "gui/GUIService.h"
#include "database/DatabaseService.h"
#include "input/InputService.h"

#include <OgreException.h>

namespace Opde {

// args are temporary!
void run(const std::string &config_name, std::string mission) {
    Root root(SERVICE_ALL);

    // logging setup
    root.logToFile("opde.log");
    root.setLogLevel(4);

    // first argument is config name
    // that includes specification of resource locations
    root.loadConfigFile(config_name);

    auto cfg_srv = GET_SERVICE(ConfigService);

    // get the resource config spec
    auto rc = cfg_srv->getParam("resource_config", "thief1.cfg");

    // enable the frame tracer?
    auto tr_ena = cfg_srv->getParam("enable_tracer", false);
    if (tr_ena.toBool())
        Tracer::getSingleton().enable(true);

    // setup resources, based on game
    // NOTE: Temporary solution here
    root.loadResourceConfig(rc.toString());

    // inform root that this is all needed for bootstrap of the services
    // (mandatory)
    root.bootstrapFinished();

    // get loop service so that we can run the main loop
    auto loop_srv = GET_SERVICE(LoopService);

    // setup some basic input bindings
    auto inp_srv = GET_SERVICE(InputService);

    // database service for mission loading
    auto db_svc = GET_SERVICE(DatabaseService);

    // register a refresh callback to db service for redraws
    // NOTE: Would like multithread later on
    db_svc->setProgressListener(
            [&](const DatabaseProgressMsg &m) {
                loop_srv->step();
            }
    );

    // request termination on escape
    inp_srv->registerCommandTrap("exit",
                                 [&](const InputEventMsg&) {
                                     loop_srv->requestTermination();
                                 });

    // non-bound mission loading command that can be called from console
    inp_srv->registerCommandTrap("load", [&](const InputEventMsg &m) {
        db_svc->load(m.params.toString(), DBM_COMPLETE);
    });

    // hardcoded, so we can exit the process and control the engine
    inp_srv->processCommand("bind esc exit");
    // show_console is implemented in GUIService, we're just binding it here
    inp_srv->processCommand("bind ` show_console");

    // any mission file requested yet?
    if (!mission.empty()) {
        db_svc->load(mission, DBM_COMPLETE);
    }

    // show console in any case
    auto gui_svc = GET_SERVICE(GUIService);
    gui_svc->showConsole();

    // loop
    if (loop_srv->requestLoopMode("AllClientsLoopMode"))
        loop_srv->run();
    else
        OPDE_EXCEPT("Cannot start AllClientsLoopMode");
}

} // namespace Opde

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT) {
    std::string scmd(strCmdLine);

    // split on space, find if we have two arguments or just one
    WhitespaceStringTokenizer wst(scmd, false); // false == obey the quotes

    std::string config_name = wst.next();

    std::string mission_file = "";

    if (!wst.end())
        mission_file = wst.next();
#else
int main(int argc, char **argv) {
    std::string config_name = "";
    std::string mission_file = "";

    if (argc >= 2)
        config_name = argv[1];

    if (argc >= 3)
        mission_file = argv[2];
#endif

    try {
        Opde::run(config_name, mission_file);
    } catch (Ogre::Exception &e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox(NULL, e.getFullDescription().c_str(),
                   "An exception has occured!",
                   MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: "
                  << e.getFullDescription().c_str() << std::endl;
#endif

    } catch (Opde::BasicException &e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox(NULL, e.getDetails().c_str(), "An exception has occured!",
                   MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getDetails().c_str()
                  << std::endl;
#endif
    }

    return 0;
}
