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
#include "bindings.h"

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
	std::string scriptName(strCmdLine);
#else
int main(int argc, char**argv)
{
	std::string scriptName = "";
	
	if (argc >= 2)
	    scriptName = argv[1];
#endif

    if (scriptName != "") {
        PythonLanguage::init();
	# TODO: Need a way to supply args to the script
        PythonLanguage::runScript(scriptName.c_str());
        PythonLanguage::term();
    } else {
	std::cerr << "opdeScript: Script name epected as a parameter!" << std::endl;
	return 1;
    }
    
    return 0;
}
