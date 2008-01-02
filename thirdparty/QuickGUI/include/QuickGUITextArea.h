#ifndef QUICKGUIMULTILINETEXTBOX_H
#define QUICKGUIMULTILINETEXTBOX_H

#include "QuickGUIPrerequisites.h"
#include "QuickGUILabelArea.h"

namespace QuickGUI
{
	class _QuickGUIExport TextArea :
		public LabelArea
	{
	public:
		TextArea(const Ogre::String& name, GUIManager* gm);
		~TextArea();

	protected:
	};
}

#endif
