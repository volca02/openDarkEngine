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
 *****************************************************************************/


#ifndef __GAMEPLAYSTATE_H
#define __GAMEPLAYSTATE_H

#include "GameState.h"
#include "OpdeSingleton.h"
#include "ConsoleFrontend.h"

#include <OgreMath.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreOverlayManager.h>


namespace Opde {

	class GamePlayState : public Singleton<GamePlayState>, public GameState, public ConsoleCommandListener {
		public:
			GamePlayState();
			virtual ~GamePlayState();

			virtual void start();
			virtual void exit();
			virtual void suspend();
			virtual void resume();

			virtual void update(unsigned long timePassed);

			virtual bool keyPressed( const OIS::KeyEvent &e );
			virtual bool keyReleased( const OIS::KeyEvent &e );
			virtual bool mouseMoved( const OIS::MouseEvent &e );
			virtual bool mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id );
			virtual bool mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id );

			virtual void commandExecuted(std::string command, std::string parameters);

		protected:
			Ogre::Root *mRoot;
			Ogre::SceneManager *mSceneMgr;
			Ogre::OverlayManager *mOverlayMgr;
			Ogre::Camera *mCamera;
			Ogre::Viewport *mViewport;

			// Movements:
			bool mForward;
			bool mBackward;
			bool mLeft;
			bool mRight;

			// Screenshot requested
			bool mScreenShot;

			// Mode display - Solid, wireframe
			bool mSceneDisplay;

			// Display portal meshes
			bool mPortalDisplay;

			bool mToLoadScreen;

			Ogre::Radian mRotX, mRotY;
			float mMoveScale;
			Ogre::Degree mRotScale;
			Ogre::Real mMoveSpeed, mRotateYFactor;
			Ogre::Degree mRotateSpeed;
			Ogre::Vector3 mTranslateVector;

			int mSceneDetailIndex;

			// Debug overlay, Temporary
			Ogre::Overlay* mDebugOverlay;
			Ogre::Overlay* mPortalOverlay;

			Ogre::RenderWindow* mWindow;

			ConsoleFrontend* mConsole;

			int mNumScreenShots;
	};
}

#endif
