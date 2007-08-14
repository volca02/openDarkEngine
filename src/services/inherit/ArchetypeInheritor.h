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

#ifndef __ARCHETYPEINHERITOR_H
#define __ARCHETYPEINHERITOR_H

#include "CachedInheritor.h"
#include "InheritService.h"

namespace Opde {
		/** Never Inheritor. This inheritor never inherits. Always uses only the existing self value for any object ID */
		class ArchetypeInheritor : public CachedInheritor {
				public:
					/// Constructor
					ArchetypeInheritor(InheritService* is);

					/// Destructor
					~ArchetypeInheritor();

                    virtual bool validate(int srcID, int dstID, unsigned int priority) const;
			};


        /** Cached inheritor factory. The inheritor produced is named "always" and will, as the name suggests,
         * inherit in all situations. */
		class ArchetypeInheritorFactory : public InheritorFactory {
				public:
					ArchetypeInheritorFactory();

					virtual std::string getName() const;

					InheritorPtr createInstance(InheritService* is) const;

                protected:
                    static std::string mName;
			};
	}

#endif
