#ifndef QUICKGUISIZE_H
#define QUICKGUISIZE_H

#include "OgrePrerequisites.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIExportDLL.h"

namespace QuickGUI
{
	class _QuickGUIExport Size
	{
	public:
		Size();
		Size(Ogre::Real Width, Ogre::Real Height);

		inline Size( const Size& s )
            : width( s.width ), height( s.height )
        {
        }

		inline Size& operator = ( const Size& s )
        {
            width = s.width;
            height = s.height;

            return *this;
        }

		inline bool operator != ( const Size& s ) const
        {
            return ( width != s.width ||
                height != s.height );
        }

		inline Size operator * ( const Ogre::Real& r ) const
        {
            return Size(width * r,height * r);
        }

		inline Size operator * ( const Size& s ) const
        {
			return Size(width * s.width,height * s.height);
        }

		inline Size operator / ( const Size& s ) const
        {
            return Size(width / s.width,height / s.height);
        }

		inline Size operator + ( const Size& s ) const
        {
            return Size(width + s.width,height + s.height);
        }

		inline Size operator - ( const Size& s ) const
        {
            return Size(width - s.width,height - s.height);
        }

		Ogre::Real width;
		Ogre::Real height;

		static const Size ZERO;
	};
}

#endif
