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
 *    $Id$
 *
 *****************************************************************************/

#include "config.h"

#include "GameStateManager.h"
#include "OpdeException.h"
#include "StringTokenizer.h"

#include <OgreException.h>

#ifndef OPDE_EXE_TARGET
#error OPDE_EXE_TARGET target not defined!
#endif


using namespace Opde;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
	std::string scmd(strCmdLine);

	// split on space, find if we have two arguments or just one
	WhitespaceStringTokenizer wst(scmd, false); // false == obey the quotes

	std::string GameType = wst.next();

	std::string missionName = "";

	if (!wst.end())
		missionName = wst.next();
#else
int main(int argc, char**argv)
{
	std::string GameType = "";
	std::string missionName = "";
	
	if (argc >= 2)
	    GameType = argv[1];
	    
	if (argc >= 3)
	    missionName = argv[2];
#endif

    // Create application object
    GameStateManager* man = new GameStateManager(GameType);

    try {
    	// if we have a mission name, supply
    	if (missionName != "")
			man->setDesiredMissionName(missionName);
			
		man->run();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif

    } catch( BasicException& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getDetails().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getDetails().c_str() << std::endl;
#endif
    }

    delete man;

    return 0;
}
