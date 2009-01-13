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

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "InputService.h"

namespace Opde {

	/** @brief GUI service - service which handles user interfaces */
	class OPDELIB_EXPORT GUIService : public Service, public DirectInputListener {
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


		protected:
			// Service initialization related methods
			bool init();

			void bootstrapFinished();

			void onRenderServiceMsg(const RenderServiceMsg& message);

			// Input related methods
			bool keyPressed( const OIS::KeyEvent &e );
			bool keyReleased( const OIS::KeyEvent &e );

			bool mouseMoved( const OIS::MouseEvent &e );
			bool mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id );
			bool mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id );

			virtual bool povMoved(const OIS::JoyStickEvent &e, int pov);
			virtual bool axisMoved(const OIS::JoyStickEvent &arg, int axis);
			virtual bool buttonPressed(const OIS::JoyStickEvent &arg, int button);
			virtual bool buttonReleased(const OIS::JoyStickEvent &arg, int button);

			InputServicePtr mInputSrv;
			RenderServicePtr mRenderSrv;

			/// Activity indicator
			bool mActive;

			/// Visibility indicator
			bool mVisible;

			RenderService::ListenerID mRenderServiceListenerID;
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
		private:
			static std::string mName;
	};
}


#endif
