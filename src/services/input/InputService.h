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


#ifndef __INPUTSERVICE_H
#define __INPUTSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "DVariant.h"
#include "InputContextMapper.h"
#include "Callback.h"

namespace Opde {

	// Damn this is just a template ok? I don't mean to use this, but there has to be a way to specify the listener type/mask interest (keypress, every key scan, key release, etc)
	// First, I have to specify if the input servis has to be stateful or stateless
	typedef enum {
			IET_KEYBOARD_PRESS,
			IET_KEYBOARD_HOLD,
			IET_KEYBOARD_RELEASE,
			IET_MOUSE_MOVE,
			IET_JOYSTICK_MOVE
	} InputEventType;

	typedef struct {
		InputEventType event
	} InputEventMsg;

	/** @brief Input service - service which handles user input, and user input mapping */
	class InputService : public Service {
		public:
			InputService(ServiceManager *manager);
			virtual ~InputService();

			/// Input command. If you call this one a line-by-line on .BND file, it should catch up and be ready to operate by the settings
			DVariant command(const std::string& command);

			// RLY?
			void setDefaultBindContext(const std::string& context) { mDefaultBindContext = context; };

			/// Set a current bind context
			void setBindContext(const std::string& context) { mCurrentBindContext = context; };

			/// Converts the given key event to a unique textual form, ready to be searched with. Some events will be converted differently - mouse/joystick f.e.
			std::string toKeyText();

			/// Definition of the command listener
			typedef Callback< InputEventMsg > Listener;
			typedef shared_ptr< Listener > ListenerPtr;

			void registerListener(ListenerPtr listener);
			void unregisterListener(ListenerPtr listener);
		protected:
            bool init();
            void bootstrapFinished();



            std::string mDefaultBindContext;
            std::string mCurrentBindContext;
	};

	/// Shared pointer to game service
	typedef shared_ptr<InputService> InputServicePtr;

	/// Factory for the GameService objects
	class InputServiceFactory : public ServiceFactory {
		public:
			InputServiceFactory();
			~InputServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

		private:
			static std::string mName;
	};
}


#endif
