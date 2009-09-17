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
 *	  $Id$
 *
 *****************************************************************************/

#include "config.h"
#include "ConsoleGUI.h"
#include "ConsoleBackend.h"
#include "RenderService.h"
#include "GUIService.h"

#include "OpdeException.h"

#include <OIS.h>
#include <string.h>


using namespace Ogre;
using namespace OIS;
using namespace std;

namespace Opde {

	ConsoleGUI::ConsoleGUI(GUIService* owner) : mIsActive(false), mOwner(owner) {
		// get handle to the font
		mFont = mOwner->getConsoleFont();
		// Core atlas that should be used to render core gui
		mAtlas = mOwner->getCoreAtlas();
		
		assert(!mFont.isNull());
		
		mDrawSrv = GET_SERVICE(DrawService);
		
		mConsoleBackend = ConsoleBackend::getSingletonPtr();

		mPosition = -1; // follow
		
		// pull out the current resolution from render service
		RenderServicePtr rs = GET_SERVICE(RenderService);
		
		const RenderWindowSize& rws = rs->getCurrentScreenSize();
		resolutionChanged(rws.width, rws.height);
	}

	ConsoleGUI::~ConsoleGUI() {
		// destroy the draw operations and the sheet
	}

	void ConsoleGUI::setActive(bool active) {
		mIsActive = active;
	}

	bool ConsoleGUI::injectKeyPress(const OIS::KeyEvent &e) {
		if (!mIsActive)
			return false;

		if (e.key == KC_RETURN) {
			ConsoleBackend::getSingleton().executeCommand(mCommand);
			mCommand = "";
		} else if (e.key == KC_BACK) {
			mCommand = mCommand.substr(0, mCommand.length()-1);
		} else if (e.key == KC_PGUP) {
			mPosition -= 28; // let two lines be the same
			mConsoleBackend->setChanged();
		} else if (e.key == KC_PGDOWN) {
			mPosition += 28;
			mConsoleBackend->setChanged();
		} else if (e.key == KC_END) {
			mPosition = -1;
			mConsoleBackend->setChanged();
		}else if (e.key == KC_HOME) {
			mPosition = 0;
			mConsoleBackend->setChanged();
		}
		else {
			string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ,./:;'-_+=[]{}()| \"\t";
			/*
			string key = "" + (char)(e.text);

			if (allowed.find(key) != string::npos) {
				mCommand += key;
			}*/

			char key[2];
			key[0]  = (char)(e.text);
			key[1] = 0;

			if (allowed.find(key) != string::npos)
				mCommand += key;
		}

		// MOD: mCommandLine->setCaption(">" + mCommand);

		return true;

	}

	void ConsoleGUI::update(int timems) {
		if (mIsActive) {
			// MOD: mConsoleOverlay->show();
			if (mConsoleBackend->getChanged()) {
				// need to update the text window
				std::vector< Ogre::String > texts;
				mConsoleBackend->pullMessages(texts, mPosition, 30);

				String text;

				std::vector< Ogre::String >::iterator it = texts.begin();

				for (;it != texts.end(); ++it) {
					text += *it + "\n";
				}

				// MOD: mConsoleText->setCaption(text);
			}
		} else {
			// MOD: mConsoleOverlay->hide();
		}
		/*
		Real scrollY = mConsoleOverlay->_getTop();

		if (mIsActive && scrollY < 0) {
			scrollY += timems/1000;

			if (scrollY > 1)
				scrollY = 1;

			mConsoleOverlay->setTop(scrollY);
		}

		if (!mIsActive && scrollY > -0.5) {
			scrollY -= timems/1000;

			if (scrollY < -0.5)
				scrollY = -0.5;

			mConsoleOverlay->setTop(scrollY);
		}*/
	}
	
	/// recalculates the number of visible lines based on the current screen size and some limit
	void ConsoleGUI::resolutionChanged(size_t width, size_t height) {
		// TODO:
	}
}
