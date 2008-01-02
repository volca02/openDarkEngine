#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIEffect.h"

namespace QuickGUI
{
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	bool Effect::update(const Ogre::Real timeElapsed)
	{
		mCurrTime += timeElapsed;
		if (mCurrTime >= mStartTime)
		{
			const bool finished = getTimeOrIsFinished();

			if (finished)
			{
				updateValue(Ogre::Real(1.0));
			}
			else
			{
				mWidget->setUnderEffect(true);
				updateValue(getTimeFactor());
			}
			return finished;
		}
		// not yet started.
		return false;
	}
	//------------------------------------------------------------------------------------------------
	Ogre::Real Effect::getTimeFactor() 
	{          
		return (mCurrTime - mStartTime) / mDuration;
	}
	//------------------------------------------------------------------------------------------------
	bool Effect::getTimeOrIsFinished()
	{
		if (mCurrTime - mStartTime > mDuration)
		{
			if (!mRepeat)
			{
				mCurrTime = mStartTime + mDuration;
				return true;
			}
			while (mCurrTime - mStartTime > mDuration)
				mCurrTime -= mDuration;
		}
		return false;
	}
	//------------------------------------------------------------------------------------------------
	Ogre::Real Effect::interpolate(const Ogre::Real start, 
								   const Ogre::Real end, 
								   const Ogre::Real timeFactor) const
	{
		Ogre::Real factor;
		switch(mType)
		{
		case EFFECT_SIN:
			factor = Ogre::Math::Sin(Ogre::Radian(timeFactor * Ogre::Math::TWO_PI));
			break;
		case EFFECT_LINEARX4:
			factor = timeFactor * timeFactor * timeFactor * timeFactor;
			break;
		case EFFECT_LINEARX2:
			factor = timeFactor * timeFactor;
			break;
		case EFFECT_LINEAR:
		default:
			factor = timeFactor;
			break;
		}
		return (factor*end + (1 - factor)*start);
	}
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void AlphaEffect::updateValue(const Ogre::Real factor)
	{
		//mWidget->setAlpha(interpolate(mStartAlpha, mEndAlpha, factor), true);
	}
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void SizeEffect::updateValue(const Ogre::Real factor)
	{
		mWidget->setSize(
			Size(interpolate(mStartSize.width, mEndSize.width, factor),
				 interpolate(mStartSize.height, mEndSize.height, factor)
			)
			);
	}
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void MoveEffect::updateValue(const Ogre::Real factor)
	{
		mWidget->setPosition(
			Point(interpolate(mStartPosition.x, mEndPosition.x, factor),
				  interpolate(mStartPosition.y, mEndPosition.y, factor)
				  )
			);
	}
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void ActivateEffect::updateValue(const Ogre::Real factor)
	{
		//mWidget->setMaterialBlendFactor(interpolate (0, 1, factor), true);
	} 
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void OverEffect::updateValue(const Ogre::Real factor)
	{
		//mWidget->setMaterialBlendFactor(interpolate (0, 1, factor), true);
	}
	//------------------------------------------------------------------------------------------------

}
