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

#include <string.h>

using namespace Ogre;
using namespace std;

// some maximal line count for the console...
#define CONSOLE_LINES 30
#define SCROLL_LINES (CONSOLE_LINES - 2)

namespace Opde {

	ConsoleGUI::ConsoleGUI(GUIService* owner) : mIsActive(false), mOwner(owner) {
		// get handle to the font
		mFont = mOwner->getConsoleFont();
		// Core atlas that should be used to render core gui
		mAtlas = mOwner->getCoreAtlas();

		mConsoleColors.push_back(ColourValue::Red);
		mConsoleColors.push_back(ColourValue(1.0f,0.7f,0.7f));
		mConsoleColors.push_back(ColourValue::White);
		mConsoleColors.push_back(ColourValue(0.8f,0.8f,0.6f));
		mConsoleColors.push_back(ColourValue(0.8f,0.8f,0.5f));
		mConsoleColors.push_back(ColourValue(0.8f,0.8f,0.3f));
		mConsoleColors.push_back(ColourValue::Blue);

		assert(!mFont.isNull());

		mDrawSrv = GET_SERVICE(DrawService);
		mInputSrv = GET_SERVICE(InputService);

		mConsoleBackend = ConsoleBackend::getSingletonPtr();

		mPosition = -1; // follow

		// TODO: Create a grey transparent rectangle below the text (needs work in draw service)

		mConsoleBackground = mDrawSrv->createRenderedRect(mAtlas);
		// mConsoleBackground->setColour(ColourValue(0.5f, 0.5f, 0.5f, 0.5f));
		mConsoleBackground->setColour(ColourValue(0.0f, 0.0f, 0.0f));
		mConsoleBackground->setZOrder(1);

		mCommandLineBackground = mDrawSrv->createRenderedRect(mAtlas);
		mCommandLineBackground->setColour(ColourValue(0.95f, 0.75f, 0.55f, 0.75f));
		mCommandLineBackground->setZOrder(1);

		mConsoleText = mDrawSrv->createRenderedLabel(mFont, "");
		mCommandLine = mDrawSrv->createRenderedLabel(mFont, ">");
		mConsoleSheet = mDrawSrv->createSheet("CONSOLE_SHEET");

		mConsoleText->setZOrder(2);
		mCommandLine->setZOrder(2);

		// show up on our sheet
		mConsoleSheet->addDrawOperation(mConsoleBackground);
		mConsoleSheet->addDrawOperation(mCommandLineBackground);
		mConsoleSheet->addDrawOperation(mConsoleText);
		mConsoleSheet->addDrawOperation(mCommandLine);

		mPosition = 0;

		// pull out the current resolution from render service
		RenderServicePtr rs = GET_SERVICE(RenderService);

		const RenderWindowSize& rws = rs->getCurrentScreenSize();
		resolutionChanged(rws.width, rws.height);
	}

	ConsoleGUI::~ConsoleGUI() {
		// destroy the draw operations and the sheet
		mDrawSrv->destroyRenderedLabel(mConsoleText);
		mConsoleText = NULL;
		mDrawSrv->destroyRenderedLabel(mCommandLine);
		mCommandLine = NULL;
		mDrawSrv->destroyRenderedRect(mConsoleBackground);
		mConsoleBackground = NULL;
		mDrawSrv->destroySheet(mConsoleSheet);

		mConsoleSheet.setNull();

		mDrawSrv.setNull();
	}

	void ConsoleGUI::setActive(bool active) {
		mIsActive = active;

		if (mIsActive)
			mDrawSrv->setActiveSheet(mConsoleSheet);
	}

	bool ConsoleGUI::injectKeyPress(unsigned int keycode) {
		if (!mIsActive)
			return false;

		if (keycode == SDLK_ESCAPE) {
			mOwner->hideConsole();
		} else if (keycode == SDLK_RETURN) {
			// add the command to the log...
			mConsoleBackend->putMessage(">" + mCommand, 6);
			// and execute
			DVariant result = mInputSrv->processCommand(mCommand);

			if (!result.toBool())
				mConsoleBackend->putMessage("Invalid command or parameters", 1);

			mCommand = "";
		} else if (keycode == SDLK_BACKSPACE) {
			mCommand = mCommand.substr(0, mCommand.length()-1);
		} else if (keycode == SDLK_PAGEUP) {
			if (mPosition >= SCROLL_LINES)
				mPosition -= SCROLL_LINES; // let two lines be the same
			else if (mPosition < 0) {
				mPosition = mConsoleBackend->getHistorySize();

				mPosition = std::max(0, mPosition - (SCROLL_LINES + SCROLL_LINES + 1));
			} else
				mPosition = 0;

			mConsoleBackend->setChanged();
		} else if (keycode == SDLK_PAGEDOWN) {
			if (mPosition >= 0)
				mPosition += SCROLL_LINES;

			// if at the end, set follow
			if ((unsigned int)(mPosition) > mConsoleBackend->getHistorySize() - SCROLL_LINES)
				mPosition = -1;

			mConsoleBackend->setChanged();
		} else if (keycode == SDLK_END) {
			mPosition = -1;
			mConsoleBackend->setChanged();
		}else if (keycode == SDLK_HOME) {
			mPosition = 0;
			mConsoleBackend->setChanged();
		}
		else {
            // not a char key?
            if (keycode & (1 << 30))
                return true;

			string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ,./:;'-_+=[]{}()| \"\t";

			char key[2];
			key[0]  = (char)(keycode);
			key[1] = 0;

			if (allowed.find(key) != string::npos)
				mCommand += key;
		}

		mCommandLine->clearText();
		mCommandLine->addText(">" + mCommand, Ogre::ColourValue::Blue);

		return true;

	}

	void ConsoleGUI::update(int timems) {
		if (mIsActive) {
			mConsoleSheet->activate();

			if (mConsoleBackend->getChanged()) {
				// need to update the text window
				std::vector< ConsoleBackend::Message > texts;
				mConsoleBackend->pullMessages(texts, mPosition, 30);

				String text;

				mConsoleText->clearText();
				std::vector< ConsoleBackend::Message >::iterator it = texts.begin();

				for (;it != texts.end(); ++it) {
					size_t level = it->first;
					if (level >= mConsoleColors.size())
						level = mConsoleColors.size() - 1;

					mConsoleText->addText(it->second, mConsoleColors[level]);
					mConsoleText->addText("\n", Ogre::ColourValue::White);
				}
			}
		} else {
			mConsoleSheet->deactivate();
		}
	}

	/// recalculates the number of visible lines based on the current screen size and some limit
	void ConsoleGUI::resolutionChanged(size_t width, size_t height) {
		// some default positioning
		// we handle maximally CONSOLE_LINES lines
		size_t screenLines = (height / mFont->getHeight()) - 1;
		if (screenLines > CONSOLE_LINES)
			screenLines = CONSOLE_LINES;

		// maximal console width
		size_t screenColumns = (width / mFont->getWidth());

		mTextClipRect.top = 0;
		mTextClipRect.left = 0;
		mTextClipRect.right = screenColumns * mFont->getWidth();
		mTextClipRect.bottom = (screenLines - 1) * mFont->getHeight();
		mTextClipRect.noClip = false;

		// background rect
		mConsoleBackground->setPosition(mTextClipRect.left, mTextClipRect.top);
		mConsoleBackground->setWidth(mTextClipRect.right);
		mConsoleBackground->setHeight((screenLines - 1) * mFont->getHeight());

		mCmdLineClipRect.top = mTextClipRect.bottom;
		mCmdLineClipRect.left = mTextClipRect.left;
		mCmdLineClipRect.right = mTextClipRect.right;
		mCmdLineClipRect.bottom = mCmdLineClipRect.top + mFont->getHeight();
		mCmdLineClipRect.noClip = false;

		mCommandLineBackground->setPosition(mCmdLineClipRect.left, mCmdLineClipRect.top);
		mCommandLineBackground->setWidth(mCmdLineClipRect.right);
		mCommandLineBackground->setHeight(mFont->getHeight());

		mConsoleText->setPosition(mTextClipRect.left, mTextClipRect.top);
		mCommandLine->setPosition(mCmdLineClipRect.left, mCmdLineClipRect.top);

		mConsoleText->setClipRect(mTextClipRect);
		mCommandLine->setClipRect(mCmdLineClipRect);
	}
}
