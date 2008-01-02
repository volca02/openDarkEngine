#ifndef QUICKGUIVERTEXBUFFER_H
#define QUICKGUIVERTEXBUFFER_H

#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgrePrerequisites.h"
#include "OgreRenderOperation.h"
#include "OgreRenderSystem.h"
#include "OgreRoot.h"
#include "OgreTextureManager.h"
#include "OgreTextureUnitState.h"

#include "QuickGUIPrerequisites.h"
#include "QuickGUIExportDLL.h"
#include "QuickGUIQuad.h"
#include "QuickGUISkinSetManager.h"
#include "QuickGUIVertex.h"

#include <list>
#include <map>
#include <utility>

namespace QuickGUI
{
	// Forward declarations
	class GUIManager;

	class _QuickGUIExport VertexBuffer
	{
	public:
		VertexBuffer(unsigned int size, GUIManager* gm);
		~VertexBuffer();

		bool empty();

		bool full();

		size_t getBufferSize();
		bool getUpdateBeforeRender();

		void render();
		void resizeVertexBuffer( const size_t numberOfVertices );

		void setData(QuadList* l);
		void setUpdateBeforeRender(bool update);

		void update();

	protected:
		GUIManager* mGUIManager;

		Ogre::RenderSystem* mRenderSystem;
		QuadList* mRenderObjectList;
		QuadList mVisibleRenderObjectList;
		bool mUpdateBeforeRender;

		// Stores the Texture of a quad, and the index of the first quad following it with a different texture.
		TextureChangeList mMaterialChangeList;

		// required to ensure textures are loaded before used
		Ogre::TextureManager* mTextureManager;

		Ogre::HardwareVertexBufferSharedPtr mVertexBuffer;
		size_t mVertexBufferSize;	// the maximum number of vertices the buffer can hold.
		size_t mVertexBufferUsage;	// the current number of vertices stored in the buffer.
		
#if VERTICES_PER_QUAD == 4
		Ogre::HardwareIndexBufferSharedPtr mIndexBuffer;
#endif

		// Every RenderSystem::_render(mRenderOperation) constitutes a batch.
		Ogre::RenderOperation mRenderOperation;

		// Cache Data
		Ogre::TextureUnitState::UVWAddressingMode mTextureAddressMode; //we cache this to save cpu time
		Ogre::LayerBlendModeEx mColorBlendMode; // we cache this to save cpu time
		Ogre::LayerBlendModeEx mAlphaBlendMode; // we cache this to save cpu time

		SkinSetManager* mSkinSetManager;

	private:
		void _initRenderState();

		// NOTE: we do not need a _clearVertexBuffer function, since we get new, blank memory when locking with HBL_DISCARD.
		void _createVertexBuffer();
		void _createIndexBuffer();

		/*
		* Define the size and data types that form a *Vertex*, to be used in the VertexBuffer.
		* NOTE: For ease of use, we define a structure QuickGUI::Vertex, with the same exact data types.
		*/
		void _declareVertexStructure();
		void _destroyVertexBuffer();
		void _renderVertexBuffer();

		void _destroyIndexBuffer();
	};
}

#endif
