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

#include <math.h>

#include "config.h"

#include "GameStateManager.h"
#include "OpdeException.h"
#include "filelog.h"
#include "logger.h"
#include "stdlog.h"

// All the services
#include "config/ConfigService.h"
#include "database/DatabaseService.h"
#include "draw/DrawService.h"
#include "game/GameService.h"
#include "inherit/InheritService.h"
#include "input/InputService.h"
#include "link/LinkService.h"
#include "loop/LoopService.h"
#include "object/ObjectService.h"
#include "property/PropertyService.h"
#include "render/RenderService.h"
#include "worldrep/WorldRepService.h"

#include "GameLoadState.h"
#include "GamePlayState.h"

#include "ProxyArchive.h"

#include "tracer.h"

#include <OgreConfigFile.h>
#include <OgreRoot.h>
#include <OgreWindowEventUtilities.h>
#include <SDL2/SDL_timer.h>

using namespace Ogre;

namespace Opde {

// The instance owner
template <> GameStateManager *Singleton<GameStateManager>::ms_Singleton = 0;

GameStateManager::GameStateManager(const std::string &GameType)
    : mStateStack(), mTerminate(false), mConsoleBackend(NULL), mRoot(NULL),
      mConfigService(NULL), mInputService(NULL), mServiceMgr(NULL) {
    mGameType = GameType;

    mRoot = new Opde::Root(SERVICE_ALL, "opde.log");
}

GameStateManager::~GameStateManager() {
    if (mInputService)
        mInputService->unsetDirectListener();

    mInputService.reset();
    mConfigService.reset();

    while (!mStateStack.empty()) {
        GameState *state = mStateStack.top();
        mStateStack.pop();

        state->exit();
    }

    delete mRoot;
}

GameStateManager &GameStateManager::getSingleton(void) {
    assert(ms_Singleton);
    return (*ms_Singleton);
}

GameStateManager *GameStateManager::getSingletonPtr(void) {
    return ms_Singleton;
}

//------------------ Main implementation ------------------
void GameStateManager::terminate() { mTerminate = true; }

void GameStateManager::pushState(GameState *state) {
    if (!mStateStack.empty()) {
        mStateStack.top()->exit();
    }

    mStateStack.push(state);

    state->start();
}

void GameStateManager::popState() {
    if (!mStateStack.empty()) {
        GameState *old = mStateStack.top();

        mStateStack.pop();

        old->exit();

        if (!mStateStack.empty()) { // There is another state in the stack
            // mStateStack.top()->start();
        } else {
            // mTerminate = true; // Terminate automatically if this state was
            // the last?
        }
    } else {
        OPDE_EXCEPT("State stack was empty, nothing could be done.",
                    "StateManager::popState");
    }
}

bool GameStateManager::run() {
    // Initialize opde logger and console backend
    // Create an ogre's root
    mOgreRoot = Ogre::Root::getSingletonPtr();

    mConsoleBackend = Opde::ConsoleBackend::getSingletonPtr();
    mConsoleBackend->putMessage("==Console Starting==");

    assert(ServiceManager::getSingletonPtr() != 0);

    if (ServiceManager::getSingletonPtr() == 0)
        LOG_FATAL("Rotten tomatoes!");

    mConfigService = GET_SERVICE(ConfigService);

    // TODO: Temporary till we come up with a commandline parsing ability
    mConfigService->setConfigPathOverride(".");

    mConfigService->loadParams("opde.cfg");

    // override the config setting
    mConfigService->setParam("game_type", mGameType);

    // To be sure it exists, and because this is just a testing code
    // and to remove the need to parse scripts,
    // we create the data-less Relation PlayerFactory here
    LinkServicePtr linksvc = GET_SERVICE(LinkService);
    linksvc->createRelation("PlayerFactory", DataStoragePtr(NULL), false);
    linksvc.reset();

    RenderServicePtr rends = GET_SERVICE(RenderService);

    // Setup resources.
    setupResources();

    // Initialise resources
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    if (mMissionName != "")
        mConfigService->setParam("mission", mMissionName);

    if (!mConfigService->hasParam("mission")) // Failback
    {
        if ((mGameType == "SS2") || (mGameType == "ss2"))
            mConfigService->setParam("mission", "earth.mis");
        else
            mConfigService->setParam("mission", "miss1.mis");
    }

    int MaxAtlasSize = 512;
    if (mConfigService->hasParam("MaxAtlasSize")) {
        MaxAtlasSize = mConfigService->getParam("MaxAtlasSize").toInt();
        MaxAtlasSize =
            1 << (int)(log((double)MaxAtlasSize) /
                       log((double)2)); // Make sure it is a exponent of two
        if ((MaxAtlasSize < 128) || (MaxAtlasSize > 4096))
            MaxAtlasSize = 512;
    }
    LOG_INFO("Max Atlas Size : %d", MaxAtlasSize);
    LightAtlas::setMaxSize(MaxAtlasSize);

    // TODO: Remove this temporary nonsense. In fact. Remove the whole class
    // this method is in!
    std::unique_ptr<GamePlayState> ps(new GamePlayState());
    std::unique_ptr<GameLoadState> ls(new GameLoadState());

    // Set default mipmap level (NB some APIs ignore this)
    TextureManager::getSingleton().setDefaultNumMipmaps(5);

    setupInputSystem();

    /* a not bulletproof but possibly useful processing of the mission config
     * parameter: The parameter is split to path and mission file name the path
     * goes to the default resource group (if not already in) the name is then
     * loaded
     */
    if (mConfigService->hasParam("mission")) {
        Variant mis = mConfigService->getParam("mission");

        // process
        String path = "", fname = mis.toString();

        StringUtil::splitFilename(mis.toString(), fname, path);

        LOG_INFO(
            "GameStateManager: Adding %s resource path. Will load mission %s",
            path.c_str(), fname.c_str());

        // now look if we need the path added into general
        /* Commented for now, causes problems for some reason...
           if (path != "")
           mRoot->addResourceLocation(path, "Dir",
           ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, false);
        */
        LOG_INFO("GameStateManager: Mission name set to %s", fname.c_str());

        mConfigService->setParam("mission", fname);
    }

    LOG_INFO("GameStateManager: Finishing bootstrap");

    mRoot->bootstrapFinished();

    ps->bootstrapFinished();

    LOG_INFO("GameStateManager: State");

    // Push the initial state
    pushState(ls.get());

    // Run the game loop
    // Main while-loop
    unsigned long lTimeCurrentFrame = 0;

    Timer *timer = mOgreRoot->getTimer();

    std::unique_ptr<Opde::Tracer> tracer(new Opde::Tracer(timer));

#warning TODO: Replace this code with propper loop manager code.

    while (!mTerminate) {
        TRACE_POINT(FRAME_START);
        TRACE_FRAME_BEGIN;

        // Calculate time since last frame and remember current time for next
        // frame
        mTimeLastFrame = lTimeCurrentFrame;
        lTimeCurrentFrame = timer->getMicroseconds();

        unsigned long lTimeSinceLastFrame = 0;
        if (lTimeCurrentFrame > mTimeLastFrame)
            lTimeSinceLastFrame = lTimeCurrentFrame - mTimeLastFrame;

        {
            TRACE_SCOPE(INPUT_HANDLING);
            // manually pull input events
            mInputService->pollEvents(lTimeSinceLastFrame / 1000000.0f);
        }

        {
            TRACE_SCOPE(STATE_UPDATE);
            // Update current state
            mStateStack.top()->update(lTimeSinceLastFrame);
        }

        {
            TRACE_SCOPE(RENDER_ONE_FRAME);
            // Render next frame
            mOgreRoot->renderOneFrame();
        }

        {
            TRACE_SCOPE(MESSAGE_PUMP);
            // Deal with platform specific issues
            Ogre::WindowEventUtilities::messagePump();
        }

        unsigned long lAfterRender = timer->getMicroseconds();
        // sleep till the end of maximal fps, for now hardcoded at
        // 300

        unsigned long diff = lAfterRender - lTimeCurrentFrame;

        // 300 FPS == 3333 useconds each
        if (diff < 3333) {
            TRACE_SCOPE(FRAME_LIMITER)
            // coarse sleep, we don't have microsecond sleep here...
            SDL_Delay((3333 - diff) / 1000);
        }

        TRACE_POINT(FRAME_END);
    }

    while (!mStateStack.empty()) {
        GameState *state = mStateStack.top();
        mStateStack.pop();

        state->exit();
    }

    return true;
}

/// Method which will define the source of resources (other than current folder)
void GameStateManager::setupResources(void) {
    // First, register the script loaders...
    // Load resource paths from config file
    String configName = "resources.cfg", GameType = "Default";

    // Load the resources according to the game type, if game type not
    // specified, load the default
    if ((mGameType == "T1") || (mGameType == "t1")) {
        configName = "thief1.cfg";
        GameType = "Thief TDP/G";
    } else if ((mGameType == "T2") || (mGameType == "t2")) {
        configName = "thief2.cfg";
        GameType = "Thief II TMA";
    } else if ((mGameType == "SS2") || (mGameType == "ss2")) {
        configName = "shock2.cfg";
        GameType = "System Shock 2";
    }

    LOG_INFO("Game type: %s", GameType.c_str());

    mRoot->loadResourceConfig(configName);
}

void GameStateManager::setupInputSystem() {
    mInputService = GET_SERVICE(InputService);

    mInputService->createBindContext("game");
    mInputService->setBindContext("game");

    mInputService->setInputMode(IM_DIRECT);
    // Commented out for now, as it is not needed
    // mInputService->loadBNDFile("dark.bnd");

    mInputService->setDirectListener(this);
}

bool GameStateManager::keyPressed(const SDL_KeyboardEvent &e) {
    if (!mStateStack.empty()) {
        mStateStack.top()->keyPressed(e);
    }
    return false;
}

bool GameStateManager::keyReleased(const SDL_KeyboardEvent &e) {
    if (!mStateStack.empty()) {
        mStateStack.top()->keyReleased(e);
    }
    return false;
}

bool GameStateManager::mouseMoved(const SDL_MouseMotionEvent &e) {
    if (!mStateStack.empty()) {
        mStateStack.top()->mouseMoved(e);
    }
    return false;
}

bool GameStateManager::mousePressed(const SDL_MouseButtonEvent &e) {
    if (!mStateStack.empty()) {
        mStateStack.top()->mousePressed(e);
    }
    return false;
}

bool GameStateManager::mouseReleased(const SDL_MouseButtonEvent &e) {
    if (!mStateStack.empty()) {
        mStateStack.top()->mouseReleased(e);
    }
    return false;
}
} // namespace Opde
