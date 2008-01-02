#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIRect.h"

namespace QuickGUI
{
	Rect::Rect() :
		x(0),
		y(0),
		width(0),
		height(0)
	{
	}

	Rect::Rect(Ogre::Real X, Ogre::Real Y, Ogre::Real Width, Ogre::Real Height) :
		x(X),
		y(Y),
		width(Width),
		height(Height)
	{
	}

	Rect::Rect(Point p, Size s) :
		x(p.x),
		y(p.y),
		width(s.width),
		height(s.height)
	{
	}

	Rect Rect::getIntersection( const Rect& r )
	{
		Rect retVal = Rect::ZERO;

		if(intersectsRect(r))
		{
			retVal.x = std::max(x,r.x);
			retVal.y = std::max(y,r.y);
			retVal.width = std::min(x + width, r.x + r.width) - retVal.x;
			retVal.height = std::min(y + height, r.y + r.height) - retVal.y;
		}

		return retVal;
	}

	bool Rect::inside(const Rect& r)
	{
		if( (x >= r.x) &&
			(y >= r.y) &&
			((x + width) <= (r.x + r.width)) &&
			((y + height) <= (r.y + r.height)) )
			return true;

		return false;
	}

	bool Rect::intersectsRect(const Rect& r)
	{
		// if our left side is greater than r's right side, or our right side is less than r's left side, intersection is not possible.
		if( (x >= (r.x + r.width)) || ((x + width) <= r.x) )
			return false;

		// if our top is greater than r's bottom, or our bottom is less than r's top, intersection is not possible.
		if( (y >= (r.y + r.height)) || ((y + height) <= r.y) )
			return false;

		// If the above conditions are not met, than there must be overlap between our dimensions and r's dimensions.
		return true;
	}

	bool Rect::isPointWithinBounds(const Point& pixelPosition)
	{
		float xPos = pixelPosition.x;
		float yPos = pixelPosition.y;

		if( (xPos < x) || (xPos > (x + width)) )
			return false;

		if( (yPos < y) || (yPos > (y + height)) )
			return false;

		return true;
	}

	const Rect Rect::ZERO( 0, 0, 0, 0 );
}
