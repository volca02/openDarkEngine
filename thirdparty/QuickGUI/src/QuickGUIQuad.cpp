#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIQuad.h"
#include "QuickGUIQuadContainer.h"
#include "QuickGUISkinSetManager.h"
#include "QuickGUIManager.h"
#include "QuickGUIUtility.h"

// for min/max
#include <algorithm>

namespace QuickGUI
{
	Quad::Quad(Widget* owner) :
		mOwner(owner),
		mGUIManager(owner->getGUIManager()),
		mQuadContainer(NULL),
		mLayer(LAYER_CHILD),
		mColorChanged(false),
		mDimensionsChanged(false),
		mMaterialChanged(false),
		mTextureCoordsChanged(false),
		mOffsetChanged(false),
		mAddedToRenderGroup(false),
		mMaterialName(""),
		mOffset(0),
		mPixelDimensions(Rect::ZERO),
		mTextureCoordinates(Ogre::Vector4(0,0,1,1)),
		mVisible(true),
		mTopColor(Ogre::ColourValue::White),
		mBottomColor(Ogre::ColourValue::White),
		mClippingWidget(NULL),
		mInheritClippingWidget(true),
		mInheritQuadLayer(true),
		mShowWithOwner(true),
		mDirectXUsed(false)
	{
		mRenderSystem = Ogre::Root::getSingleton().getRenderSystem();

		// Determine if dirext X renderer used.
		if(mRenderSystem->getName().find("Direct") != Ogre::String::npos)
			mDirectXUsed = true;

		_updateVertexColor();
	}

	Quad::Quad(GUIManager* gm) :
		mOwner(NULL),
		mGUIManager(gm),
		mQuadContainer(NULL),
		mLayer(LAYER_CHILD),
		mColorChanged(false),
		mDimensionsChanged(false),
		mMaterialChanged(false),
		mTextureCoordsChanged(false),
		mOffsetChanged(false),
		mAddedToRenderGroup(false),
		mMaterialName(""),
		mOffset(0),
		mPixelDimensions(Rect::ZERO),
		mTextureCoordinates(Ogre::Vector4(0,0,1,1)),
		mVisible(true),
		mTopColor(Ogre::ColourValue::White),
		mBottomColor(Ogre::ColourValue::White),
		mClippingWidget(NULL),
		mDimensionsViaClipping(mPixelDimensions),
		mTextureCoordinatesViaClipping(mTextureCoordinates)
	{
		mRenderSystem = Ogre::Root::getSingleton().getRenderSystem();
		_updateVertexColor();
	}

	Quad::~Quad()
	{
		if(mAddedToRenderGroup) 
			removeFromRenderObjectGroup();
	}

	void Quad::_clip()
	{
		if(mClippingWidget == NULL)
		{
			mDimensionsViaClipping = mPixelDimensions;
			_computeVertices();
			mTextureCoordinatesViaClipping = mTextureCoordinates;
			_updateTextureCoords();
			_notifyQuadContainerNeedsUpdate();
			return;
		}

		Rect clippingRect = getClippingRect();

		if(mPixelDimensions.inside(clippingRect))
		{
			mDimensionsViaClipping = mPixelDimensions;
			_computeVertices();

			mTextureCoordinatesViaClipping = mTextureCoordinates;
			_updateTextureCoords();

			if(mOwner->isVisible())
				mVisible = true;
			else
				mVisible = false;

			_notifyQuadContainerNeedsUpdate();
		}
		else if(mPixelDimensions.intersectsRect(clippingRect))
		{
			mDimensionsViaClipping = mPixelDimensions.getIntersection(clippingRect);
			_computeVertices();

			// calculate distance between top/bottom and left/right of the UV coords.
			Ogre::Real uvWidth = mTextureCoordinates.z - mTextureCoordinates.x;
			Ogre::Real uvHeight = mTextureCoordinates.w - mTextureCoordinates.y;
			
			mTextureCoordinatesViaClipping.x = mTextureCoordinates.x + (((mDimensionsViaClipping.x - mPixelDimensions.x) / mPixelDimensions.width) * uvWidth);
			mTextureCoordinatesViaClipping.y = mTextureCoordinates.y + (((mDimensionsViaClipping.y - mPixelDimensions.y) / mPixelDimensions.height) * uvHeight);
			mTextureCoordinatesViaClipping.z = mTextureCoordinates.z - ((((mPixelDimensions.x + mPixelDimensions.width) - (mDimensionsViaClipping.x + mDimensionsViaClipping.width)) / mPixelDimensions.width) * uvWidth);
			mTextureCoordinatesViaClipping.w = mTextureCoordinates.w - ((((mPixelDimensions.y + mPixelDimensions.height) - (mDimensionsViaClipping.y + mDimensionsViaClipping.height)) / mPixelDimensions.height) * uvHeight);
			
			_updateTextureCoords();

			if(mOwner->isVisible())
				mVisible = true;
			else
				mVisible = false;

			_notifyQuadContainerNeedsUpdate();
		}
		else // Quad is outside clipping region.
		{
			mDimensionsViaClipping = mPixelDimensions;
			_computeVertices();

			mTextureCoordinatesViaClipping = mTextureCoordinates;
			_updateTextureCoords();

			mVisible = false;
			_notifyQuadContainerNeedsUpdate();
		}
	}

	void Quad::_computeVertices()
	{
		if(mGUIManager == NULL)
			return;

		// Convert pixel values into absolute coordinates.
		Ogre::Real viewportWidth = mGUIManager->getViewportWidth();
		Ogre::Real viewportHeight = mGUIManager->getViewportHeight();

		Rect absDimensions(
			mDimensionsViaClipping.x / viewportWidth,
			mDimensionsViaClipping.y / viewportHeight,
			mDimensionsViaClipping.width / viewportWidth,
			mDimensionsViaClipping.height / viewportHeight);

		/* Convert positions into -1, 1 coordinate space (homogeneous clip space).
			- Left / right is simple range conversion
			- Top / bottom also need inverting since y is upside down - this means
			that top will end up greater than bottom and when computing texture
			coordinates, we have to flip the v-axis (ie. subtract the value from
			1.0 to get the actual correct value).
		*/
		Ogre::Real left = (absDimensions.x * 2) - 1;
		Ogre::Real right = left + (absDimensions.width * 2);
		Ogre::Real top = -((absDimensions.y * 2) - 1);
		Ogre::Real bottom = top - (absDimensions.height * 2);

		// TRIANGLE 1
		mVertices[0].pos = Ogre::Vector3(left,  bottom, 0.f);	// left-bottom
		mVertices[1].pos = Ogre::Vector3(right, bottom, 0.f);	// right-bottom

		// TRIANGLE 2
		mVertices[2].pos = Ogre::Vector3(right, top, 0.f);		// right-top
		mVertices[3].pos = Ogre::Vector3(left,  top, 0.f);		// left-top

		
		/* Reference code, back when the index buffer was not used, and there were 6 vertices per quad.
		
		// TRIANGLE 1
		mVertices[0].pos = Ogre::Vector3(left,bottom,0);	// left-bottom
		mVertices[1].pos = Ogre::Vector3(right,bottom,0);	// right-bottom
		mVertices[2].pos = Ogre::Vector3(left,top,0);		// left-top

		// TRIANGLE 2
		mVertices[3].pos = Ogre::Vector3(right,bottom,0);	// right-bottom
		mVertices[4].pos = Ogre::Vector3(right,top,0);		// right-top
		mVertices[5].pos = Ogre::Vector3(left,top,0);		// left-top

		*/
	}

	void Quad::_notifyAddedToRenderObjectGroup()
	{
		mAddedToRenderGroup = true;
	}

	void Quad::_notifyChangesHandled()
	{
		mColorChanged = false;
		mDimensionsChanged = false;
		mMaterialChanged = false;
		mTextureCoordsChanged = false;
		mOffsetChanged = false;
	}

	void Quad::_notifyRemovedFromQuadContainer()
	{
		mAddedToRenderGroup = false;
	}

	void Quad::_notifyQuadContainer(QuadContainer* container)
	{
		if(mAddedToRenderGroup && (mQuadContainer != NULL))
			removeFromRenderObjectGroup();

		mQuadContainer = container;

		if(mQuadContainer != NULL)
			addToRenderObjectGroup();
	}

	void Quad::_notifyQuadContainerNeedsUpdate()
	{
		// notify QuadContainer of change (if exists)
		if( (mQuadContainer != NULL) && (mAddedToRenderGroup) ) 
		{
			switch(mLayer)
			{
			case Quad::LAYER_CHILD:
				mQuadContainer->notifyChildRenderableChanged(this);	
				break;
			case Quad::LAYER_MENU:
				mQuadContainer->notifyMenuRenderableChanged(this);
				break;
			}
		}
	}

	void Quad::_updateTextureCoords()
	{
		const Ogre::Vector4 actualTextureCoords(mTextureCoordinatesViaClipping);

		// TRIANGLE 1 : bot-left, bot-right, top-left vertices
		mVertices[0].uv = Ogre::Vector2(actualTextureCoords.x,actualTextureCoords.w);
		mVertices[1].uv = Ogre::Vector2(actualTextureCoords.z,actualTextureCoords.w);
		// TRIANGLE 2 : bot-right, top-right, top-left vertices
		mVertices[2].uv = Ogre::Vector2(actualTextureCoords.z,actualTextureCoords.y);
		mVertices[3].uv = Ogre::Vector2(actualTextureCoords.x,actualTextureCoords.y);

		/* Reference code, back when the index buffer was not used, and there were 6 vertices per quad.

		// TRIANGLE 1 : bot-left, bot-right, top-left vertices
		mVertices[0].uv = Ogre::Vector2(actualTextureCoords.x,actualTextureCoords.w);
		mVertices[1].uv = Ogre::Vector2(actualTextureCoords.z,actualTextureCoords.w);
		mVertices[2].uv = Ogre::Vector2(actualTextureCoords.x,actualTextureCoords.y);
		// TRIANGLE 2 : bot-right, top-right, top-left vertices
		mVertices[3].uv = Ogre::Vector2(actualTextureCoords.z,actualTextureCoords.w);
		mVertices[4].uv = Ogre::Vector2(actualTextureCoords.z,actualTextureCoords.y);
		mVertices[5].uv = Ogre::Vector2(actualTextureCoords.x,actualTextureCoords.y);

		*/
	}

	void Quad::_updateVertexColor()
	{
		// TRIANGLE 1
		mRenderSystem->convertColourValue(mBottomColor,&( mVertices[0].color ));
		mRenderSystem->convertColourValue(mBottomColor,&( mVertices[1].color ));
		// TRIANGLE 2
		mRenderSystem->convertColourValue(mTopColor,&( mVertices[2].color ));
		mRenderSystem->convertColourValue(mTopColor,&( mVertices[3].color ));

		/* Reference code, back when the index buffer was not used, and there were 6 vertices per quad.

		// TRIANGLE 1
		mRenderSystem->convertColourValue(mBottomColor,&( mVertices[0].color ));
		mRenderSystem->convertColourValue(mBottomColor,&( mVertices[1].color ));
		mRenderSystem->convertColourValue(mTopColor,&( mVertices[2].color ));
		// TRIANGLE 2
		mRenderSystem->convertColourValue(mBottomColor,&( mVertices[3].color ));
		mRenderSystem->convertColourValue(mTopColor,&( mVertices[4].color ));
		mRenderSystem->convertColourValue(mTopColor,&( mVertices[5].color ));

		*/
	}

	void Quad::addToRenderObjectGroup()
	{
		if( mAddedToRenderGroup || (mQuadContainer == NULL) ) 
			return;

		switch(mLayer)
		{
		case Quad::LAYER_CHILD:
			mQuadContainer->addChildRenderable(this);
			break;
		case Quad::LAYER_MENU:
			mQuadContainer->addMenuRenderable(this);
			break;
		}

		mAddedToRenderGroup = true;
	}

	void Quad::setClippingWidget(Widget* w)
	{
		mClippingWidget = w;
	}

	bool Quad::dimensionsChanged()
	{
		return mDimensionsChanged;
	}

	Rect Quad::getClippingRect()
	{
		return Rect(mClippingWidget->getScreenPosition() + mClippingWidget->getScrollOffset(), mClippingWidget->getSize());
	}

	Widget* Quad::getClippingWidget()
	{
		return mClippingWidget;
	}

	Rect Quad::getDimensions()
	{
		return mPixelDimensions;
	}

	bool Quad::getInheritClippingWidget()
	{
		return mInheritClippingWidget;
	}

	bool Quad::getInheritLayer()
	{
		return mInheritQuadLayer;
	}

	void Quad::setInheritLayer(bool inherit)
	{
		mInheritQuadLayer = inherit;
	}

	Quad::Layer Quad::getLayer()
	{
		return mLayer;
	}

	Ogre::String Quad::getMaterialName()
	{
		return mMaterialName;
	}

	Ogre::Real Quad::getOpacity()
	{
		return mTopColor.a;
	}

	bool Quad::getShowWithOwner()
	{
		return mShowWithOwner;
	}

	bool Quad::isPointWithinBounds(const Point& pixelPosition)
	{
		const float xPos = pixelPosition.x;
		if( (xPos < mPixelDimensions.x) || (xPos > (mPixelDimensions.x + mPixelDimensions.width)) )
			return false;

		const float yPos = pixelPosition.y;
		if( (yPos < mPixelDimensions.y) || (yPos > (mPixelDimensions.y + mPixelDimensions.height)) )
			return false;

		return true;
	}

	Vertex* Quad::getVertices()
	{
		return mVertices;
	}

	const int Quad::getOffset() const
	{
		return mOffset;
	}

	Point Quad::getPosition()
	{
		return Point(mPixelDimensions.x,mPixelDimensions.y);
	}

	Size Quad::getSize()
	{
		return Size(mPixelDimensions.width,mPixelDimensions.height);
	}

	void Quad::removeFromRenderObjectGroup()
	{
		if( !mAddedToRenderGroup || (mQuadContainer == NULL) ) return;

		switch(mLayer)
		{
		case Quad::LAYER_CHILD:
			mQuadContainer->removeChildRenderable(this);
			break;
		case Quad::LAYER_MENU:
			mQuadContainer->removeMenuRenderable(this);
			break;
		}

		mAddedToRenderGroup = false;
	}

	void Quad::setColor(const Ogre::ColourValue& color)
	{
		setColor(color,color);
	}

	void Quad::setColor(const Ogre::ColourValue& topColor, const Ogre::ColourValue& botColor)
	{
		mTopColor = topColor;
		mBottomColor = botColor;

		mColorChanged = true;

		_updateVertexColor();

		_notifyQuadContainerNeedsUpdate();
	}

	void Quad::setDimensions(const Rect& pixelDimensions)
	{
		mPixelDimensions = pixelDimensions;
		mDimensionsChanged = true;

		_clip();
	}

	void Quad::setGUIManager(GUIManager* gm)
	{
		mGUIManager = gm;
		_computeVertices();
	}

	void Quad::setHeight(Ogre::Real pixelHeight)
	{
		mPixelDimensions.height = pixelHeight;

		mDimensionsChanged = true;

		_clip();
	}

	void Quad::setInheritClippingWidget(bool inherit)
	{
		mInheritClippingWidget = inherit;
	}

	void Quad::setLayer(Layer l)
	{
		if( (l == mLayer) && mAddedToRenderGroup ) return;

		removeFromRenderObjectGroup();
		mLayer = l;
		addToRenderObjectGroup();
	}

	void Quad::setMaterial(const Ogre::String& materialName)
	{
		mMaterialName = materialName;
		mMaterialChanged = true;

		_notifyQuadContainerNeedsUpdate();
	}

	void Quad::setOffset(int offset)
	{
		mOffset = offset;

		mOffsetChanged = true;

		_notifyQuadContainerNeedsUpdate();
	}

	void Quad::setOpacity(Ogre::Real opacity)
	{
		mTopColor.a = opacity;
		mBottomColor.a = opacity;

		mColorChanged = true;

		_updateVertexColor();

		_notifyQuadContainerNeedsUpdate();
	}

	void Quad::setPosition(const Point& pixelPosition)
	{
		mPixelDimensions.x = pixelPosition.x;
		mPixelDimensions.y = pixelPosition.y;

		mDimensionsChanged = true;

		_clip();
	}

	void Quad::setShowWithOwner(bool showWithOwner)
	{
		mShowWithOwner = showWithOwner;
	}

	void Quad::setSize(const Size& pixelSize)
	{
		mPixelDimensions.width = pixelSize.width;
		mPixelDimensions.height = pixelSize.height;

		mDimensionsChanged = true;

		_clip();
	}

	void Quad::setTextureCoordinates(const Ogre::Vector4& textureCoordinates)
	{
		// DirectX interprets UV coords from a different location than OpenGL, so
		// we have to modify the coordinates to get the same results.
/*		if(mDirectXUsed)
		{
			Ogre::Real xVal = 0.5 / mPixelDimensions.width; 
			Ogre::Real yVal = 0.5 / mPixelDimensions.height;

			mTextureCoordinates = Ogre::Vector4(
				textureCoordinates.x + xVal,
				textureCoordinates.y + yVal,
				textureCoordinates.z + xVal,
				textureCoordinates.w + yVal);
		}
		else */
			mTextureCoordinates = textureCoordinates;

		mTextureCoordsChanged = true;

		_clip();
	}

	void Quad::setTextureCoordinates(const Ogre::FloatRect& textureCoordinates)
	{
		setTextureCoordinates(Ogre::Vector4(textureCoordinates.left,textureCoordinates.top,textureCoordinates.right,textureCoordinates.bottom));
	}

	void Quad::setVisible(bool visible)
	{
		mVisible = visible;

		// Only clip when becoming visible
		if(mVisible)
			_clip();
		else
			_notifyQuadContainerNeedsUpdate();
	}

	void Quad::setWidth(Ogre::Real pixelWidth)
	{
		mPixelDimensions.width = pixelWidth;

		mDimensionsChanged = true;

		_clip();
	}

	void Quad::setXPosition(Ogre::Real pixelX)
	{
		mPixelDimensions.x = pixelX;

		mDimensionsChanged = true;

		_clip();
	}

	void Quad::setYPosition(Ogre::Real pixelY)
	{
		mPixelDimensions.y = pixelY;

		mDimensionsChanged = true;

		_clip();
	}

	bool Quad::textureChanged()
	{
		return mMaterialChanged;
	}

	bool Quad::textureExists(const Ogre::String& textureName)
	{
		if(textureName.empty())
			return false;

		if(Utility::textureExistsOnDisk(textureName))
			return true;

		if(Ogre::TextureManager::getSingletonPtr()->resourceExists(textureName)) 
			return true;

		if(SkinSetManager::getSingletonPtr()->embeddedInSkinSet(textureName))
			return true;

		return false;
	}

	void Quad::updateClippingWidget()
	{
		if(mOwner->getParentPanel() != NULL)
			mClippingWidget = mOwner->getParentPanel();
		else if(mOwner->getParentWindow() != NULL)
			mClippingWidget = mOwner->getParentWindow();
		else if(mOwner->getParentSheet() != NULL)
			mClippingWidget = mOwner->getParentSheet();
		else
			mClippingWidget = mOwner;
	}


	bool Quad::offsetChanged()
	{
		return mOffsetChanged;
	}
}
