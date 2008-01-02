# CMake file for QuickGUI
# Edited by Volca from the original files (Modified for static linkage only)

SET ( QUICKGUI_SRCS 
	QuickGUIBorder.cpp
      	QuickGUIButton.cpp
	QuickGUICheckBox.cpp 
	QuickGUIComboBox.cpp
	QuickGUIConfigScriptParser.cpp
	QuickGUIConsole.cpp 
	QuickGUIEffect.cpp
	QuickGUIHorizontalScrollBar.cpp
	QuickGUIHorizontalTrackBar.cpp
	QuickGUIImage.cpp 
	QuickGUILabel.cpp 
	QuickGUILabelArea.cpp
	QuickGUIList.cpp 
	QuickGUIManager.cpp 
	QuickGUIMenuLabel.cpp
	QuickGUIMouseCursor.cpp 
	QuickGUINStateButton.cpp 
	QuickGUIPanel.cpp 
	QuickGUIPoint.cpp 
	QuickGUIPrecompiledHeaders.cpp
	QuickGUIProgressBar.cpp 
	QuickGUIQuad.cpp 
	QuickGUIRect.cpp
#	QuickGUIRenderBatch.cpp
#	QuickGUIRenderComponent.cpp
#	QuickGUIRenderManager.cpp
	QuickGUIRoot.cpp
	QuickGUIScrollPane.cpp
	QuickGUISheet.cpp 
	QuickGUISize.cpp 
	QuickGUISkinSet.cpp
	QuickGUISkinSetManager.cpp
	QuickGUITextArea.cpp
	QuickGUITextBox.cpp 
	QuickGUITextHelper.cpp
	QuickGUIText.cpp 
	QuickGUITitleBar.cpp 
	QuickGUIUtility.cpp
	QuickGUIVertexBuffer.cpp 
	QuickGUIVerticalScrollBar.cpp
	QuickGUIVerticalTrackBar.cpp
	QuickGUIWidget.cpp 
	QuickGUIWindow.cpp 
	)

INCLUDE_DIRECTORIES (../include ${OGRE_INCLUDE_DIR})

LINK_DIRECTORIES (${OGRE_LIB_DIR})

ADD_LIBRARY (QuickGUI ${QUICKGUI_SRCS})

