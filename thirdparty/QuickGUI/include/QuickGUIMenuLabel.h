#ifndef QUICKGUIMENULABEL_H
#define QUICKGUIMENULABEL_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUIButton.h"
#include "QuickGUIImage.h"
#include "QuickGUILabel.h"
#include "QuickGUIExportDLL.h"

namespace QuickGUI
{
	class _QuickGUIExport MenuLabel :
		public Label
	{
	public:
		enum Layout
		{
			LAYOUT_LEFT_TO_RIGHT	=  0,
			LAYOUT_RIGHT_TO_LEFT
		};
	public:
		MenuLabel(const Ogre::String& name, GUIManager* gm);

		Ogre::String getButtonSkin();
		Ogre::String getIconMaterial();

		void setButtonSkin(const Ogre::String& skinName);
		void setIconMaterial(const Ogre::String& materialName);
		template<typename T> void setMouseButtonUpOnButtonHandler(void (T::*function)(const EventArgs&), T* obj)
		{
			if(mButton == NULL)
				return;

			mButton->addEventHandler(EVENT_MOUSE_BUTTON_UP,new MemberFunctionPointer<T>(function,obj),obj);
		}
		void setMouseButtonUpOnButtonHandler(MemberFunctionSlot* function);

	protected:
		virtual ~MenuLabel();

		Layout mLayout;

		Image* mIcon;
		Ogre::String mIconMaterialName;

		Button* mButton;
		Ogre::String mButtonTextureName;

		void onSizeChanged(const EventArgs& args);
	};
}

#endif
