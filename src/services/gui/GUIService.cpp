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
 *	  $Id$
 *
 *****************************************************************************/

#include "GUIService.h"
#include "DarkCommon.h"
#include "OpdeServiceManager.h"
#include "ServiceCommon.h"
#include "StringTokenizer.h"
#include "config/ConfigService.h"
#include "draw/DrawService.h"
#include "gui/ConsoleGUI.h"
#include "loop/LoopService.h"
#include "render/RenderService.h"

using namespace std;
using namespace Ogre;

namespace Opde {

/*-----------------------------------------------------*/
/*-------------------- GUIService ---------------------*/
/*-----------------------------------------------------*/
template <> const size_t ServiceImpl<GUIService>::SID = __SERVICE_ID_GUI;

GUIService::GUIService(ServiceManager *manager, const std::string &name)
    : ServiceImpl<Opde::GUIService>(manager, name),
      mActive(false),
      mVisible(false),
      mActiveSheet(),
      mConsole(),
      mCoreAtlas(NULL),
      mConsoleFont(NULL),
      mRenderServiceListenerID(0),
      mInputSrv(NULL),
      mRenderSrv(NULL)
{

    mLoopClientDef.id = LOOPCLIENT_ID_GUI;
    mLoopClientDef.mask = LOOPMODE_GUI;
    mLoopClientDef.name = mName;
    mLoopClientDef.priority = LOOPCLIENT_PRIORITY_GUI;
}

// -----------------------------------
GUIService::~GUIService() { }

// -----------------------------------
void GUIService::setActive(bool active) {
    /* What do we do here?
       if set to true, we set direct input mode and show cursor
       if set to false, we set mapped input mode and hide the cursor
    */
    assert(!mInputSrv.isNull());

    if (active) {
        mInputSrv->setInputMode(IM_DIRECT);
    } else {
        mInputSrv->setInputMode(IM_MAPPED);
    }

    mActive = active;
}

// -----------------------------------
void GUIService::setVisible(bool visible) {
    mVisible = visible;

    if (!mVisible) {
        if (mConsole->isActive())
            hideConsole();

        setActive(false);
    }
}

// -----------------------------------
bool GUIService::init() { return true; }

// -----------------------------------
void GUIService::bootstrapFinished() {
    mInputSrv = GET_SERVICE(InputService);
    mRenderSrv = GET_SERVICE(RenderService);
    mDrawSrv = GET_SERVICE(DrawService);
    mConfigSrv = GET_SERVICE(ConfigService);
    mLoopSrv = GET_SERVICE(LoopService);

    assert(mRenderSrv->getSceneManager());

    // handler for direct listener
    mInputSrv->setDirectListener(this);

    // Register as a listener for the resolution changes
    RenderService::ListenerPtr renderServiceListener(
        new ClassCallback<RenderServiceMsg, GUIService>(
            this, &GUIService::onRenderServiceMsg));
    mRenderServiceListenerID =
        mRenderSrv->registerListener(renderServiceListener);

    InputService::ListenerPtr showConsoleListener(
        new ClassCallback<InputEventMsg, GUIService>(
            this, &GUIService::onShowConsole));

    mInputSrv->registerCommandTrap("show_console", showConsoleListener);

    mLoopSrv->addLoopClient(this);

    // Core rendering sources
    mConfigSrv->setParamDescription(
        "console_font_name",
        "Font file name of the base console font used for debugging");
    mConfigSrv->setParamDescription("console_font_group",
                                    "Resource group of the base console font");

    DVariant tmp;
    mConsoleFontName = "font.fon";

    if (mConfigSrv->getParam("console_font_name", tmp)) {
        mConsoleFontName = tmp.toString();
    } else {
        LOG_ERROR("console_font_name parameter not set, using default '%s'!",
                  mConsoleFontName.c_str());
    }

    mConsoleFontGroup = "General";
    if (mConfigSrv->getParam("console_font_group", tmp)) {
        mConsoleFontGroup = tmp.toString();
    } else {
        LOG_ERROR("console_font_group parameter not set, using default '%s'!",
                  mConsoleFontGroup.c_str());
    }

    // TODO: Do we need palette for the console font?
    mCoreAtlas = mDrawSrv->createAtlas();
    mDrawSrv->setFontPalette(ePT_Default);
    mConsoleFont =
        mDrawSrv->loadFont(mCoreAtlas, mConsoleFontName, mConsoleFontGroup);

    // create the console.
    mConsole.reset(new ConsoleGUI(this));
}

// -----------------------------------
void GUIService::shutdown() {
    if (mInputSrv) {
        mInputSrv->unsetDirectListener();
    }

    if (mRenderSrv)
        mRenderSrv->unregisterListener(mRenderServiceListenerID);

    mLoopSrv->removeLoopClient(this);

    mInputSrv->unregisterCommandTrap("show_console");

    mConsole.reset();

    mRenderSrv.reset();
    mInputSrv.reset();
    mDrawSrv.reset();
    mConfigSrv.reset();
    mLoopSrv.reset();
}

// -----------------------------------
bool GUIService::keyPressed(const SDL_KeyboardEvent &e) {
    if (mConsole && mConsole->isActive()) {
        mConsole->injectKeyPress(e.keysym.sym);
        return true;
    } else {
        // TODO: Inject into the focussed GUI object
        return true;
    }
}

// -----------------------------------
bool GUIService::keyReleased(const SDL_KeyboardEvent &e) { return true; }

// -----------------------------------
bool GUIService::mouseMoved(const SDL_MouseMotionEvent &e) { return true; }

// -----------------------------------
bool GUIService::mousePressed(const SDL_MouseButtonEvent &e) { return true; }

// -----------------------------------
bool GUIService::mouseReleased(const SDL_MouseButtonEvent &e) { return true; }

// -----------------------------------
void GUIService::onRenderServiceMsg(const RenderServiceMsg &message) {
    // Inform the console about the resolution change
    if (mConsole)
        mConsole->resolutionChanged(message.size.width, message.size.height);

    // TODO: Inform the GUI components as well?
}

// -----------------------------------
void GUIService::onShowConsole(const InputEventMsg &iem) {
    if (mConsole && mConsole->isActive()) {
        hideConsole();
    } else {
        showConsole();
    }
}

// -----------------------------------
void GUIService::showConsole() {
    if (!mConsole)
        return;

    // backup the previous situation
    mCBActive = mActive;
    mCBSheet = mDrawSrv->getActiveSheet();
    mCBVisible = mVisible;

    // Activate the console
    mConsole->setActive(true);

    // activate GUI
    setActive(true);
    setVisible(true);
}

// -----------------------------------
void GUIService::hideConsole() {
    if (!mConsole)
        return;

    mConsole->setActive(false);

    // restore the previous situation
    mDrawSrv->setActiveSheet(mCBSheet);
    setActive(mCBActive);
    setVisible(mCBVisible);
}

// -----------------------------------
void GUIService::loopStep(float deltaTime) {
    // hmm. console update here
    if (mConsole)
        mConsole->update(deltaTime * 1000);
}

// -----------------------------------
FontDrawSourcePtr GUIService::getConsoleFont() const { return mConsoleFont; }

// -----------------------------------
TextureAtlasPtr GUIService::getCoreAtlas() const { return mCoreAtlas; }

//-------------------------- Factory implementation
std::string GUIServiceFactory::mName = "GUIService";

GUIServiceFactory::GUIServiceFactory() : ServiceFactory(){};

const std::string &GUIServiceFactory::getName() { return mName; }

const uint GUIServiceFactory::getMask() { return SERVICE_RENDERER; }

const size_t GUIServiceFactory::getSID() { return GUIService::SID; }

Service *GUIServiceFactory::createInstance(ServiceManager *manager) {
    return new GUIService(manager, mName);
}

} // namespace Opde
