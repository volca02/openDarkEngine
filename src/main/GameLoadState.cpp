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
 *
 *
 *    $Id$
 *
 *****************************************************************************/

#include "config.h"

#include "GameLoadState.h"
#include "GamePlayState.h"
#include "GameStateManager.h"

#include "game/GameService.h"
#include "render/RenderService.h"

#include "logger.h"

#include <OgreFontManager.h>
#include <OgreOverlay.h>
#include <OgreTextAreaOverlayElement.h>

using namespace Ogre;

namespace Opde {

template <> GameLoadState *Singleton<GameLoadState>::ms_Singleton = 0;

GameLoadState::GameLoadState()
    : mSceneMgr(NULL), mOverlayMgr(NULL), mLoaded(false) {
    mRoot = Ogre::Root::getSingletonPtr();
    mOverlayMgr = OverlayManager::getSingletonPtr();
    mServiceMgr = ServiceManager::getSingletonPtr();

    mConfigService = GET_SERVICE(ConfigService);
}

GameLoadState::~GameLoadState() {}

void GameLoadState::start() {
    Variant fnttst;

    LOG_INFO("LoadState: Starting");

    mSceneMgr = mRoot->getSceneManager("DarkSceneManager");

    mSceneMgr->clearSpecialCaseRenderQueues();
    mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
    mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);

    RenderServicePtr renderSrv = GET_SERVICE(RenderService);

    mCamera = renderSrv->getDefaultCamera();
    mViewport = renderSrv->getDefaultViewport();

    LOG_INFO("LoadState: Started");

    mLoaded = false;
    mFirstTime = true;
}

void GameLoadState::exit() {
    LOG_INFO("LoadState: Exiting");
    // TODO: Replace with drawservice code mLoadingOverlay->hide();

    LOG_INFO("LoadState: Exited");

    if (mLoaded) {
        pushState(GamePlayState::getSingletonPtr());
    }
}

void GameLoadState::suspend() { LOG_INFO("LoadState: Suspend?!"); }

void GameLoadState::resume() {
    LOG_INFO("LoadState: Resume?!");
    mLoaded = false;
    mFirstTime = true;
}

void GameLoadState::update(unsigned long timePassed) {
    if (!mFirstTime && !mLoaded) {
        unsigned long start_ms =
            Ogre::Root::getSingleton().getTimer()->getMilliseconds();

        mRoot->renderOneFrame();

        GameServicePtr gsvc = GET_SERVICE(GameService);

        std::string misFile = mConfigService->getParam("mission");

        gsvc->load(misFile);

        mLoaded = true;

        LOG_INFO("Loading took %10.2f seconds",
                 (Ogre::Root::getSingleton().getTimer()->getMilliseconds() -
                  start_ms) /
                     1000.0f);

        popState(); // Hardcoded, so no escape key is needed
    }

    mFirstTime = false;
}

bool GameLoadState::keyPressed(const SDL_KeyboardEvent &e) { return false; }

bool GameLoadState::keyReleased(const SDL_KeyboardEvent &e) {
    if (e.keysym.sym == SDLK_ESCAPE) {
        // requestTermination();
        if (mLoaded)
            popState();
    }
    return false;
}

bool GameLoadState::mouseMoved(const SDL_MouseMotionEvent &e) { return false; }

bool GameLoadState::mousePressed(const SDL_MouseButtonEvent &e) {
    return false;
}

bool GameLoadState::mouseReleased(const SDL_MouseButtonEvent &e) {
    return false;
}

GameLoadState &GameLoadState::getSingleton() {
    assert(ms_Singleton);
    return *ms_Singleton;
}

GameLoadState *GameLoadState::getSingletonPtr() { return ms_Singleton; }
} // namespace Opde
