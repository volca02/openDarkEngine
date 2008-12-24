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
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __NEVERINHERITOR_H
#define __NEVERINHERITOR_H

#include "InheritService.h"

namespace Opde {
		/** Never Inheritor. This inheritor never inherits. Always uses only the existing self value for any object ID */
		class NeverInheritor : public Inheritor {
				public:
					/// Constructor
					NeverInheritor(const InheritorFactory* fac, InheritService* is);

					/// Destructor
					~NeverInheritor();

					/// @see Inheritor::setImplements
					virtual void setImplements(int objID, bool impl);

					/// @see Inheritor::getImplements
					virtual bool getImplements(int objID) const;

					/// @see Inheritor::getEffectiveID
					virtual int getEffectiveID(int srcID) const;

					/// @see Inheritor::validate
                    virtual bool validate(int srcID, int dstID, unsigned int priority) const;

					/// @see Inheritor::valueChanged
					virtual void valueChanged(int objID, const std::string& field, const DVariant& value);

					/// @see Inheritor::clear
                    virtual void clear();

				protected:
					/// InheritService reference
					InheritService* mInheritService;

 					/// Map of implementing object IDs - id->true means object implements the inherited property
					typedef std::map <int, bool> ImplementsMap;

					ImplementsMap mImplements;
			};


        /** Never inheritor factory. The inheritor produced is named "never" and will, as the name suggests,
         * never inherit. */
		class NeverInheritorFactory : public InheritorFactory {
				public:
					NeverInheritorFactory();

					virtual std::string getName() const;

					Inheritor* createInstance(InheritService* is) const;

                protected:
                    static std::string mName;
			};
	}

#endif
