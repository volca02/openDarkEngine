#ifndef QUICKGUIEFFECT_H
#define QUICKGUIEFFECT_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIManager.h"

namespace QuickGUI
{
	//------------------------------------------------------------------------------------------------
	class _QuickGUIExport Effect
	{
	public:
		enum EffectInterpolatorType
		{
			EFFECT_LINEAR = 0,
			EFFECT_LINEARX2,
			EFFECT_LINEARX4,
			EFFECT_SIN
		};
	public:
		Effect(Widget * const widget, const Ogre::Real duration, 
			const Ogre::Real startTime, const Ogre::Real currTime = 0, const bool repeat = false,
			EffectInterpolatorType type = EFFECT_LINEARX4) : 
		mWidget(widget), 
			mStartTime(startTime), 
			mDuration(duration), 
			mRepeat(repeat),
			mCurrTime(currTime),
			mType(type)
		{
			assert (duration != 0);
		}
		virtual ~Effect(){};

		virtual void updateValue(const Ogre::Real factor) = 0;

		bool update(const Ogre::Real timeElapsed);
		Ogre::Real getTimeFactor() ;
		bool getTimeOrIsFinished();
		Ogre::Real linearInterpolate(const Ogre::Real start, 
			const Ogre::Real end, 
			const Ogre::Real timeFactor) const;
		Ogre::Real interpolate(const Ogre::Real start, 
			const Ogre::Real end, 
			const Ogre::Real timeFactor) const;

	protected:
		Widget * const mWidget;
		const Ogre::Real mStartTime; 
		const Ogre::Real mDuration;
		const bool mRepeat;

		Ogre::Real mCurrTime;
		const EffectInterpolatorType mType;
	};
	//------------------------------------------------------------------------------------------------
	class _QuickGUIExport AlphaEffect : public Effect
	{
	public:

		AlphaEffect(Widget * const widget, 
			const Ogre::Real duration, 
			const Ogre::Real startAlpha,
			const Ogre::Real endAlpha,
			const Ogre::Real startTime, 
			const Ogre::Real currTime = 0, 
			const bool repeat = false,
			EffectInterpolatorType type = EFFECT_LINEARX4) : 
		Effect(widget, duration, startTime,  currTime, repeat), 
			mStartAlpha(startAlpha), 
			mEndAlpha(endAlpha)
		{
			assert(mStartAlpha >= 0.0 && mStartAlpha <= 1.0);
			assert(mEndAlpha >= 0.0 && mEndAlpha <= 1.0);
		}
		virtual ~AlphaEffect(){};

		void updateValue(const Ogre::Real factor);
	private:
		const Ogre::Real mStartAlpha;
		const Ogre::Real mEndAlpha;
	};
	//------------------------------------------------------------------------------------------------
	class _QuickGUIExport MoveEffect : public Effect
	{
	public:
		MoveEffect(Widget * const widget, 
			const Ogre::Real duration, 
			const Point &startPosition,
			const Point &endPosition,
			const Ogre::Real startTime, 
			const Ogre::Real currTime = 0, 
			const bool repeat = false,
			EffectInterpolatorType type = EFFECT_LINEARX4) : 
		Effect(widget, duration, startTime,  currTime, repeat), 
			mStartPosition(startPosition), 
			mEndPosition(endPosition)
		{}
		virtual ~MoveEffect(){};
		void updateValue(const Ogre::Real factor);

	private:
		const Point mStartPosition;
		const Point mEndPosition;
	};
	//------------------------------------------------------------------------------------------------
	class _QuickGUIExport SizeEffect : public Effect
	{
	public:
		SizeEffect(Widget * const widget, 
			const Ogre::Real duration, 
			const Size &startSize,
			const Size &endSize,
			const Ogre::Real startTime, 
			const Ogre::Real currTime = 0, 
			const bool repeat = false,
			EffectInterpolatorType type = EFFECT_LINEARX4) : 
		Effect(widget, duration, startTime, currTime, repeat), 
			mStartSize(startSize), 
			mEndSize(endSize)
		{}
		virtual ~SizeEffect(){};
		void updateValue(const Ogre::Real factor);

	private:
		const Size mStartSize;
		const Size mEndSize;
	};
	//------------------------------------------------------------------------------------------------
	class _QuickGUIExport ActivateEffect : public Effect
	{
	public:
		ActivateEffect(Widget * const widget, 
			const Ogre::Real duration, 
			const Ogre::Real startTime, 
			const Ogre::Real currTime = 0, 
			const bool repeat = false,
			EffectInterpolatorType type = EFFECT_LINEARX4): 
		Effect(widget, duration, startTime, currTime, repeat)
		{};
		virtual ~ActivateEffect(){};
		void updateValue(const Ogre::Real factor);

	};
	//------------------------------------------------------------------------------------------------
	class _QuickGUIExport OverEffect : public Effect
	{
	public:
		OverEffect(Widget * const widget, 
			const Ogre::Real duration, 
			const Ogre::Real startTime, 
			const Ogre::Real currTime = 0, 
			const bool repeat = false,
			EffectInterpolatorType type = EFFECT_LINEARX4): 
		Effect(widget, duration, startTime, currTime, repeat)
		{};
		virtual ~OverEffect(){};
		void updateValue(const Ogre::Real factor);

	};
	//------------------------------------------------------------------------------------------------
}

#endif
