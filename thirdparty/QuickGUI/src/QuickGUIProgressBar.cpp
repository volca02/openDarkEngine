#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIProgressBar.h"
#include "QuickGUIWindow.h"

namespace QuickGUI
{
	ProgressBar::ProgressBar(const Ogre::String& name, GUIManager* gm) :
		Widget(name,gm),
		mInitialPixelOffset(0),
		mProgress(1.0),
		mFillDirection(FILLS_NEGATIVE_TO_POSITIVE),
		mClippingEdge(CLIP_LEFT_BOTTOM)
	{
		mWidgetType = TYPE_PROGRESSBAR;
		mSkinComponent = ".progressbar";
		mSize = Size(100,25);

		if( mSize.width > mSize.height )
			mLayout = LAYOUT_HORIZONTAL;
		else
			mLayout = LAYOUT_VERTICAL;

		mBarPanel = _createQuad();
		mBarPanel->setPosition(getScreenPosition());
		mBarPanel->setSize(mSize);
		mBarPanel->setOffset(mOffset+1);
		mBarPanel->_notifyQuadContainer(mQuadContainer);

		mBarMaterial = Ogre::MaterialManager::getSingleton().create(mInstanceName + ".barMaterial",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* p = mBarMaterial->getTechnique(0)->getPass(0);
		p->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		p->setLightingEnabled(false);
		p->setCullingMode(Ogre::CULL_CLOCKWISE);
		Ogre::TextureUnitState* tus = p->createTextureUnitState();
		mBarMaterial->load();
	}

	ProgressBar::~ProgressBar()
	{
		EventHandlerArray::iterator it;
		for( it = mOnProgressChangedHandlers.begin(); it != mOnProgressChangedHandlers.end(); ++it )
			delete (*it);
		mOnProgressChangedHandlers.clear();

		mBarMaterial.setNull();
	}

	void ProgressBar::_getBarExtents()
	{
		mBarMinWidth = static_cast<int>(mBarImage.getWidth());
		mBarMaxWidth = 0;
		mBarMinHeight = static_cast<int>(mBarImage.getHeight());
		mBarMaxHeight = 0;

		const Ogre::PixelBox& pb = mBarImage.getPixelBox();

		// Pointers to the pixel data of the bar, and the destination image
		Ogre::uint8* ptr = static_cast<Ogre::uint8*>(pb.data);

		// iterate through pixel by pixel, to determine the min/max bar bounds.
		for( int i = 0; i < static_cast<int>(mBarImage.getHeight()); i++ )
		{
			for( int j = 0; j < static_cast<int>(mBarImage.getWidth()); j++ )
			{
				// skip R,B,G channels
				ptr += 3;
				// check for non zero alpha value
				if(*ptr++ > 0)
				{
					if( i < mBarMinHeight )
						mBarMinHeight = i;
					if( i > mBarMaxHeight )
						mBarMaxHeight = i;
					if( j < mBarMinWidth )
						mBarMinWidth = j;
					if( j > mBarMaxWidth )
						mBarMaxWidth = j;
				}
			}
		}
	}

	void ProgressBar::_modifyBarTexture(Ogre::Real progress)
	{
		Ogre::Image barContainer;
		barContainer.load(mSkinName + mSkinComponent + mSkinExtension, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		// Hardware Pixel Buffer for moving pixels around, and correctly creating the barTexture.
		Ogre::HardwarePixelBufferSharedPtr buf = mBarTexture->getBuffer();

		// if 100 percent, just copy over the entire bar image.
		if(progress >= 0.99)
		{
			buf->blitFromMemory(mBarImage.getPixelBox(),Ogre::Image::Box(0, 0, mBarImage.getWidth(), mBarImage.getHeight()));
			return;
		}

		// At this point we are less than 100% progress, so we will have to manipulate the bar texture.

		// First, we want to make the barTexture appear like the barContainer. (looks like 0 progress)
		buf->blitFromMemory(barContainer.getPixelBox(),Ogre::Image::Box(0, 0, barContainer.getWidth(), barContainer.getHeight()));

		// if progress is too small, there is no bar texture to create.
		if(progress <= 0.01)
			return;

		int barTextureWidth = (mBarMaxWidth - mBarMinWidth);
		int barTextureHeight = (mBarMaxHeight - mBarMinHeight);

		Ogre::PixelBox srcBox;
		Ogre::Image::Box dstBox;

		// Fill the progressbar according to specified fill direction
		if(mLayout == LAYOUT_HORIZONTAL)
		{
			// Calculated pixel width of the progress bar.
			int barWidth = (barTextureWidth * progress);
			// Calculated pixel width of the space not occupied by the progress bar.
			int spaceWidth = (barTextureWidth - (progress * barTextureWidth));

			// FILLS_LEFT_TO_RIGHT
			if(mFillDirection == FILLS_NEGATIVE_TO_POSITIVE)
			{
				if(mClippingEdge == CLIP_LEFT_BOTTOM)
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box((mBarMinWidth + spaceWidth), 0, mBarMaxWidth, barContainer.getHeight()));
					dstBox = Ogre::Image::Box(mBarMinWidth, 0, (mBarMinWidth + barWidth), barContainer.getHeight());
				}
				// CLIP_RIGHT_TOP
				else
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box(mBarMinWidth, 0, (mBarMinWidth + barWidth), barContainer.getHeight()));
					dstBox = Ogre::Image::Box(mBarMinWidth, 0, (mBarMinWidth + barWidth), barContainer.getHeight());
				}
			}
			// FILLS_RIGHT_TO_LEFT
			else
			{
				if(mClippingEdge == CLIP_LEFT_BOTTOM)
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box(mBarMinWidth, 0, (mBarMinWidth + barWidth), barContainer.getHeight()));
					dstBox = Ogre::Image::Box((mBarMinWidth + spaceWidth), 0, mBarMaxWidth, barContainer.getHeight());
				}
				// CLIP_RIGHT_TOP
				else
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box((mBarMaxWidth - barWidth), 0, mBarMaxWidth, barContainer.getHeight()));
					dstBox = Ogre::Image::Box((mBarMinWidth + spaceWidth), 0, mBarMaxWidth, barContainer.getHeight());
				}
			}
		}
		else // mLayout == LAYOUT_VERTICAL
		{
			// Calculated pixel height of the progress bar.
			int barHeight = (barTextureHeight * progress);
			// Calculated pixel height of the space not occupied by the progress bar.
			int spaceHeight = (barTextureHeight - (progress * barTextureHeight));

			// FILLS_BOTTOM_TO_TOP
			if(mFillDirection == FILLS_NEGATIVE_TO_POSITIVE)
			{
				if(mClippingEdge == CLIP_LEFT_BOTTOM)
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box(0, mBarMinHeight, barContainer.getWidth(), (mBarMinHeight + barHeight)));
					dstBox = Ogre::Image::Box(0, mBarMinHeight + spaceHeight, barContainer.getWidth(), mBarMaxHeight);
				}
				// CLIP_RIGHT_TOP
				else
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box(0, (mBarMaxHeight - barHeight), barContainer.getWidth(), mBarMaxHeight));
					dstBox = Ogre::Image::Box(0, mBarMinHeight + spaceHeight, barContainer.getWidth(), mBarMaxHeight);
				}
			}
			// FILLS_TOP_TO_BOTTOM
			else
			{
				if(mClippingEdge == CLIP_LEFT_BOTTOM)
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box(0, mBarMaxHeight - barHeight, barContainer.getWidth(), mBarMaxHeight));
					dstBox = Ogre::Image::Box(0, mBarMinHeight, barContainer.getWidth(), (mBarMinHeight + barHeight));
				}
				// CLIP_RIGHT_TOP
				else
				{
					srcBox = mBarImage.getPixelBox().getSubVolume(Ogre::Box(0, mBarMinHeight, barContainer.getWidth(), (mBarMinHeight + barHeight)));
					dstBox = Ogre::Image::Box(0, mBarMinHeight, barContainer.getWidth(), (mBarMinHeight + barHeight));
				}
			}
		}

		buf->blitFromMemory(srcBox,dstBox);

		// Pointer to the pixel buffer of the bar texture, at full progress.
		Ogre::uint8* refPtr = static_cast<Ogre::uint8*>((mBarImage.getPixelBox()).data);
		// Pointer to the pixel buffer of the currently created bar progress texture.
		buf->lock(Ogre::HardwareBuffer::HBL_NORMAL);
		Ogre::uint8* bufPtr = static_cast<Ogre::uint8*>((buf->getCurrentLock()).data);

		// iterate through pixel by pixel, to Make sure the newly created bar texture does not exceed original bar bounds.
		for( int i = 0; i < static_cast<int>(mBarImage.getHeight()); ++i )
		{
			for( int j = 0; j < static_cast<int>(mBarImage.getWidth()); ++j )
			{
				// skip R,B,G channels
				refPtr += 3;
				bufPtr += 3;

				// If the original bar texture was transparent at this pixel, make the new texture's pixel transparent.
				// Even thought I'm using TAM_CLAMP for my textures, I am finding wrap around in the progress bar.. so the edges
				// of the bar are made transparent.  A Hack, but the only way I can get past this issue currently.
				if( (*refPtr == 0) ||
					// 2 pixel border
					(i == dstBox.top) || (i == dstBox.top + 1) ||
					(i == dstBox.bottom) || (i == dstBox.bottom - 1) ||
					(j == dstBox.left) || (j == dstBox.left + 1) ||
					(j == dstBox.right) || (j == dstBox.right - 1) )
					*bufPtr = 0;

				++refPtr;
				++bufPtr;
			}
		}

		buf->unlock();
	}

	void ProgressBar::addOnProgressChangedEventHandler(MemberFunctionSlot* function)
	{
		mOnProgressChangedHandlers.push_back(function);
	}

	Ogre::Real ProgressBar::getProgress()
	{
		return mProgress;
	}

	void ProgressBar::onProgressChanged(const WidgetEventArgs& e)
	{
		EventHandlerArray::iterator it;
		for( it = mOnProgressChangedHandlers.begin(); it != mOnProgressChangedHandlers.end(); ++it )
			(*it)->execute(e);
	}

	void ProgressBar::onPositionChanged(const EventArgs& args)
	{
		Widget::onPositionChanged(args);
		mBarPanel->setPosition(getScreenPosition());
	}

	void ProgressBar::onSizeChanged(const EventArgs& args)
	{
		mBarPanel->setSize(mSize);
	}

	void ProgressBar::redraw()
	{
		Widget::redraw();
		mBarPanel->setPosition(mQuad->getPosition());
	}

	void ProgressBar::setClippingEdge(ClippingEdge e)
	{
		mClippingEdge = e;
		setProgress(mProgress);
	}

	void ProgressBar::setFillDirection(FillDirection d)
	{
		mFillDirection = d;
		setProgress(mProgress);
	}

	void ProgressBar::setInitialPixelOffset(unsigned int width)
	{
		mInitialPixelOffset = width;
	}

	void ProgressBar::setLayout(Layout l)
	{
		mLayout = l;
		setProgress(mProgress);
	}

	void ProgressBar::setProgress(Ogre::Real progress)
	{
		// Check to make sure we get acceptable values
		if(progress >= 0.99) 
			mProgress = 1.0;
		else if(progress < 0.01) 
			mProgress = 0.0;
		else 
			mProgress = progress;

		// update the bar's texture to match the progress.
		_modifyBarTexture(progress);

		// fire event and call any user defined handlers
		WidgetEventArgs e(this);
		e.handled = false;
		onProgressChanged(e);
	}

	void ProgressBar::setSkin(const Ogre::String& skinName, bool recursive)
	{
		Widget::setSkin(skinName,recursive);

		SkinSet* ss = SkinSetManager::getSingleton().getSkinSet(skinName);
		if(ss == NULL)
			return;

		mSkinExtension = ss->getImageExtension();
		mBarTextureName = mSkinName + mSkinComponent + ".bar" + mSkinExtension;
		mBarImage.load(mBarTextureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		_getBarExtents();

		Ogre::TextureManager* tm = Ogre::TextureManager::getSingletonPtr();
		// resource may already exist, for example, creating the texture, clearing all gui, then creating gui again.
		if( tm->resourceExists(mInstanceName + ".bar.temp") )
		{
			tm->remove(mInstanceName + ".bar.temp");
		}
		
		mBarTexture = Ogre::TextureManager::getSingletonPtr()->createManual(mInstanceName + ".bar.temp", 
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			Ogre::TEX_TYPE_2D,
			(Ogre::uint)mWidgetImage->getWidth(),
			(Ogre::uint)mWidgetImage->getHeight(),
			0, 
			Ogre::PF_B8G8R8A8,
			Ogre::TU_STATIC);

		Ogre::TextureUnitState* tus = mBarMaterial->getBestTechnique(0)->getPass(0)->getTextureUnitState(0);
		tus->setTextureName(mBarTexture->getName());

		setProgress(1.0);

		mBarPanel->setMaterial(mBarMaterial->getName());
	}
}
