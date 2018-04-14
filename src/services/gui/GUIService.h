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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __GUISERVICE_H
#define __GUISERVICE_H

#include "config.h"

#include "ServiceCommon.h"
#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "input/InputService.h"
#include "draw/DrawService.h"
#include "config/ConfigService.h"
#include "gui/ConsoleGUI.h"

namespace Opde {

	/** @brief GUI service - service which handles user interfaces
	  * @note This class also handles console and show_console input command */
	class OPDELIB_EXPORT GUIService : public ServiceImpl<GUIService>, public DirectInputListener, public LoopClient {
		public:
			GUIService(ServiceManager *manager, const std::string& name);
			virtual ~GUIService();

			/** Set's the activeness state for GUI.
			* Either GUI gets input, or the mapped way for input is realized. @see InputService
			* @note if active, cursor is shown, hidden otherwise.
			* @param active If true, the GUI is active, cursor is shown, and all inputs go into GUI.
			*/
			void setActive(bool active);

			/** Sets the visibility state of the gui. Note: This differs from activeness in that Active
			* GUI can be Invisible and the other way round.
			* @param visible If true, the active sheet is set to show(), if false, set to hide()
			*/
			void setVisible(bool visible);

			/** Returns the pre-prepared console font. Used to render console and some debug info.
			*/
			FontDrawSourcePtr getConsoleFont() const;

			/** Returns the atlas containing the core render sources - mouse cursor, console font, etc.
			* Use this to render various debug overlays and stuff like that. */
			TextureAtlasPtr getCoreAtlas() const;

			/// Hides the console window
			void hideConsole();

			/// Shows the console window
			void showConsole();

		protected:
			// Service initialization related methods
			bool init();

			void bootstrapFinished();

			void shutdown();

			void onRenderServiceMsg(const RenderServiceMsg& message);

			void onShowConsole(const InputEventMsg& iem);

			/// Loop step event
			void loopStep(float deltaTime);

			// Input related methods
			virtual bool keyPressed(const SDL_KeyboardEvent &e);
			virtual bool keyReleased(const SDL_KeyboardEvent &e);

			virtual bool mouseMoved(const SDL_MouseMotionEvent &e);
			virtual bool mousePressed(const SDL_MouseButtonEvent &e);
			virtual bool mouseReleased(const SDL_MouseButtonEvent &e);

		private:
			/// Activity indicator
			bool mActive;

			/// Visibility indicator
			bool mVisible;

			/// Currently visible sheet
			DrawSheetPtr mActiveSheet;

			/// Console frontend - GUI render of the console
			ConsoleGUI* mConsole;

			// --- Console backup ---
			/// Console Backup - activeness flag
			bool mCBActive;

			/// Console Backup - visibility flag
			bool mCBVisible;

			/// Console Backup - prev. active sheet
			DrawSheetPtr mCBSheet;

			// --- Core rendering ---
			/// Core rendering atlas. Various overlays, mouse cursor, etc...
			TextureAtlasPtr mCoreAtlas;

			/// Console font - automatically loaded upon bootstrap finish...
			FontDrawSourcePtr mConsoleFont;

			/// Console font name
			std::string mConsoleFontName;

			/// Console font group
			std::string mConsoleFontGroup;

			/// Render service listener ID (for resolution changes)
			RenderService::ListenerID mRenderServiceListenerID;

			/// Input service ptr - used for input handling
			InputServicePtr mInputSrv;

			/// Render service ptr - used for various utilitary needs
			RenderServicePtr mRenderSrv;

			/// Loop service ptr - used for animation and other things
			LoopServicePtr mLoopSrv;

			/// Draw service ptr - used for gui rendering itself
			DrawServicePtr mDrawSrv;

			/// Config service ptr - used for font and cursor settings, etc.
			ConfigServicePtr mConfigSrv;
	};

	/// Shared pointer to a GUI service
	typedef shared_ptr<GUIService> GUIServicePtr;

	/// Factory for the GUIService objects
	class OPDELIB_EXPORT GUIServiceFactory : public ServiceFactory {
		public:
			GUIServiceFactory();
			~GUIServiceFactory() {};

			/** Creates a GUIService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

                        virtual const uint getMask();

			virtual const size_t getSID();
		private:
			static std::string mName;
	};
}


#endif
