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

#ifndef __MESSAGESOURCE_H
#define __MESSAGESOURCE_H

namespace Opde {

    /** Message Source - a message sending class template.
    * M stands for the message type sended
    * L stands for the Listener type used */
    template <typename M, typename L> class MessageSource {
        protected:
            /// A set of listeners
			typedef typename std::set< L* > Listeners;

			/// Listeners for the link changes
			Listeners mListeners;

            /// Sends a message to all listeners
            void broadcastMessage(const M& msg) const {
                typename Listeners::const_iterator it = mListeners.begin();

                for (; it != mListeners.end(); ++it) {
                    // Call the method on the listener pointer
                    ((*it)->listener->*(*it)->method)(msg);
                }
            }

        public:
			/** Registers a listener.
			* @param listener A pointer to L
			* @note The same pointer has to be supplied to the unregisterListener in order to succeed with unregistration
			*/
			void registerListener(L* listener) {
                mListeners.insert(listener);
            }

			/** Unregisters a listener.
			* @param listener A pointer to L
			* @note The pointer has to be the same as the one supplied to the registerListener (not a big deal, just supply a pointer to a member variable)
			*/
			void unregisterListener(L* listener) {
                mListeners.erase(listener);
            }
    };

}

#endif
