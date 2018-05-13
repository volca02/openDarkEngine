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

#ifndef __GAMEPLAYSTATE_H
#define __GAMEPLAYSTATE_H

#include "config.h"

#include "ConsoleCommandListener.h"
#include "GameState.h"
#include "OpdeSingleton.h"
#include "config/ConfigService.h"
#include "draw/DrawService.h"
#include "link/LinkCommon.h"
#include "link/LinkService.h"

#include <OgreMath.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>

namespace Opde {

class GamePlayState : public Singleton<GamePlayState>,
                      public GameState,
                      public ConsoleCommandListener {
public:
    GamePlayState();
    virtual ~GamePlayState();

    virtual void start();
    virtual void exit();
    virtual void suspend();
    virtual void resume();

    virtual void update(unsigned long timePassed);

    bool keyPressed(const SDL_KeyboardEvent &e);
    bool keyReleased(const SDL_KeyboardEvent &e);

    bool mouseMoved(const SDL_MouseMotionEvent &e);
    bool mousePressed(const SDL_MouseButtonEvent &e);
    bool mouseReleased(const SDL_MouseButtonEvent &e);

    virtual void commandExecuted(std::string command, std::string parameters);

    virtual void bootstrapFinished();
    void onLinkPlayerFactoryMsg(const LinkChangeMsg &msg);

    static GamePlayState &getSingleton();
    static GamePlayState *getSingletonPtr();

protected:
    Ogre::Root *mRoot;
    Ogre::SceneManager *mSceneMgr;
    Ogre::Camera *mCamera;
    Ogre::Viewport *mViewport;

    // Movements:
    bool mForward;
    bool mBackward;
    bool mLeft;
    bool mRight;

    // Screenshot requested
    bool mScreenShot;

    // Shadows toggle
    bool mShadows;

    // Mode display - Solid, wireframe
    bool mSceneDisplay;

    // Display portal meshes
    bool mPortalDisplay;

    // debug enabled
    bool mDebug;

    bool mToLoadScreen;

    Ogre::Radian mRotX, mRotY;
    float mMoveScale;
    Ogre::Degree mRotScale;
    Ogre::Real mMoveSpeed, mRotateYFactor;
    Ogre::Degree mRotateSpeed;
    Ogre::Vector3 mTranslateVector;

    int mSceneDetailIndex;

    Ogre::RenderWindow *mWindow;

    int mNumScreenShots;
    // config service
    ConfigServicePtr mConfigService;

    RenderedLabel *mRl1, *mRl2;

private:
    /// Direct link to the player factory relation
    RelationPtr mPlayerFactoryRelation;
    /// Handle to the link service
    LinkServicePtr mLinkService;
    int mStartingPointObjID;
    /// Link (Relation player factory) listener registration ID
    MessageListenerID mPlayerFactoryListenerID;

    DrawServicePtr mDrawService;
};
} // namespace Opde

#endif
