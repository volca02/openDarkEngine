#include "QuickGUIPrecompiledHeaders.h"

#include "QuickGUIVertexBuffer.h"
#include "QuickGUIManager.h"

namespace QuickGUI
{
	VertexBuffer::VertexBuffer(unsigned int size, GUIManager* gm) :
		mGUIManager(gm),
		mUpdateBeforeRender(false),
		mVertexBufferSize(size),
		mVertexBufferUsage(0),
		mRenderObjectList(0)
	{
		mSkinSetManager = SkinSetManager::getSingletonPtr();
		mRenderSystem = Ogre::Root::getSingleton().getRenderSystem();
		mTextureManager = Ogre::TextureManager::getSingletonPtr();
		_createVertexBuffer();
		_createIndexBuffer();

		// Initialise blending modes to be used. We use these every frame, so we'll set them up now to save time later.
		mColorBlendMode.blendType	= Ogre::LBT_COLOUR;
		mColorBlendMode.source1		= Ogre::LBS_TEXTURE;
		mColorBlendMode.source2		= Ogre::LBS_DIFFUSE;
		mColorBlendMode.operation	= Ogre::LBX_MODULATE;

		mAlphaBlendMode.blendType	= Ogre::LBT_ALPHA;
		mAlphaBlendMode.source1		= Ogre::LBS_TEXTURE;
		mAlphaBlendMode.source2		= Ogre::LBS_DIFFUSE;
		mAlphaBlendMode.operation	= Ogre::LBX_MODULATE;

		mTextureAddressMode.u = Ogre::TextureUnitState::TAM_CLAMP;
		mTextureAddressMode.v = Ogre::TextureUnitState::TAM_CLAMP;
		mTextureAddressMode.w = Ogre::TextureUnitState::TAM_CLAMP;
	}

	VertexBuffer::~VertexBuffer()
	{
		_destroyVertexBuffer();
		_destroyIndexBuffer();
	}

	void VertexBuffer::_createVertexBuffer()
	{
		mRenderOperation.vertexData = new Ogre::VertexData();
		mRenderOperation.vertexData->vertexStart = 0;
		mVertexBufferUsage = 0;

		_declareVertexStructure();

		// Create the Vertex Buffer, using the Vertex Structure we previously declared in _declareVertexStructure.
		mVertexBuffer = Ogre::HardwareBufferManager::getSingleton( ).createVertexBuffer(
			mRenderOperation.vertexData->vertexDeclaration->getVertexSize(0), // declared Vertex used
			mVertexBufferSize,
			Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
			false );

		// Bind the created buffer to the renderOperation object.  Now we can manipulate the buffer, and the RenderOp keeps the changes.
		mRenderOperation.vertexData->vertexBufferBinding->setBinding( 0, mVertexBuffer );
		mRenderOperation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
		mRenderOperation.useIndexes = false;
	}

	void VertexBuffer::_createIndexBuffer()
	{
		mRenderOperation.useIndexes = true;
		mRenderOperation.indexData = new Ogre::IndexData();
		mRenderOperation.indexData->indexStart = 0;
		
		// Create the Index Buffer
		size_t indexCount = (mVertexBufferSize / 4) * 6;

		mIndexBuffer = Ogre::HardwareBufferManager::getSingleton( ).createIndexBuffer(
			((mVertexBufferSize > 655534) ? Ogre::HardwareIndexBuffer::IT_32BIT: Ogre::HardwareIndexBuffer::IT_16BIT),
			indexCount,
			Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
			false);
		mRenderOperation.indexData->indexBuffer = mIndexBuffer;
	}

	void VertexBuffer::_declareVertexStructure()
	{
		Ogre::VertexDeclaration* vd = mRenderOperation.vertexData->vertexDeclaration;

		// Add position - Ogre::Vector3 : 4 bytes per float * 3 floats = 12 bytes

		vd->addElement( 0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION );

		// Add color - Ogre::RGBA : 8 bits per channel (1 byte) * 4 channels = 4 bytes

		vd->addElement( 0, Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT3 ), Ogre::VET_COLOUR, Ogre::VES_DIFFUSE );

		// Add texture coordinates - Ogre::Vector2 : 4 bytes per float * 2 floats = 8 bytes

		vd->addElement( 0, Ogre::VertexElement::getTypeSize( Ogre::VET_FLOAT3 ) +
						   Ogre::VertexElement::getTypeSize( Ogre::VET_COLOUR ),
						   Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES );

		/* Our structure representing the Vertices used in the buffer (24 bytes):
			struct Vertex
			{
				Ogre::Vector3 pos;
				Ogre::RGBA color;
				Ogre::Vector2 uv;
			};
		*/
	}

	void VertexBuffer::_destroyVertexBuffer()
	{
		delete mRenderOperation.vertexData;
		mRenderOperation.vertexData = NULL;
		mVertexBuffer.setNull();
	}

	void VertexBuffer::_destroyIndexBuffer()
	{
		delete mRenderOperation.indexData;
		mRenderOperation.indexData = NULL;
		mIndexBuffer.setNull();
	}

	void VertexBuffer::_initRenderState()
	{
		// make sure we're rendering to the correct viewport
		mRenderSystem->_setViewport(mGUIManager->getViewport());

		// set-up matrices
		mRenderSystem->_setWorldMatrix(Ogre::Matrix4::IDENTITY);
		mRenderSystem->_setViewMatrix(Ogre::Matrix4::IDENTITY);
		mRenderSystem->_setProjectionMatrix(Ogre::Matrix4::IDENTITY);

		// initialise render settings
		mRenderSystem->setLightingEnabled(false);
		mRenderSystem->_setDepthBufferParams(false, false);
		mRenderSystem->_setDepthBias(0, 0);
		mRenderSystem->_setCullingMode(Ogre::CULL_CLOCKWISE);
		mRenderSystem->_setFog(Ogre::FOG_NONE);
		mRenderSystem->_setColourBufferWriteEnabled(true, true, true, true);
		mRenderSystem->unbindGpuProgram(Ogre::GPT_FRAGMENT_PROGRAM);
		mRenderSystem->unbindGpuProgram(Ogre::GPT_VERTEX_PROGRAM);
		mRenderSystem->setShadingType(Ogre::SO_GOURAUD);
		mRenderSystem->_setPolygonMode(Ogre::PM_SOLID);

		// initialise texture settings
		mRenderSystem->_setTextureCoordCalculation(0, Ogre::TEXCALC_NONE);
		mRenderSystem->_setTextureCoordSet(0, 0);
		//mRenderSystem->_setTextureUnitFiltering(0, Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);
		mRenderSystem->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_POINT);
		mRenderSystem->_setTextureAddressingMode(0, mTextureAddressMode);
		mRenderSystem->_setTextureMatrix(0, Ogre::Matrix4::IDENTITY);
		mRenderSystem->_setAlphaRejectSettings(Ogre::CMPF_ALWAYS_PASS, 0);
		mRenderSystem->_setTextureBlendMode(0, mColorBlendMode);
		mRenderSystem->_setTextureBlendMode(0, mAlphaBlendMode);
		mRenderSystem->_disableTextureUnitsFrom(1);

		// enable alpha blending
		mRenderSystem->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
	}


	void VertexBuffer::_renderVertexBuffer()
	{
		if( mVisibleRenderObjectList.empty() ) 
			return;

		bool shadowsEnabled = mGUIManager->getViewport()->getShadowsEnabled();
		mGUIManager->getViewport()->setShadowsEnabled(false);

		/*
		* Since mRenderList is sorted by zOrder and by Texture, we can send quads with similar textures into one renderOperation.
		* Everything rendered in one _render call will receive the texture set previously by _setTexture.
		*/
		unsigned int quadCounter = 0;

		size_t indexOffset = 0;
		mRenderOperation.vertexData->vertexStart = 0;

		for(TextureChangeList::iterator materialChangeIt = mMaterialChangeList.begin(); materialChangeIt != mMaterialChangeList.end(); ++materialChangeIt)
		{
			// tell the render operation how  many vertices to read. and where
			{
				const unsigned int textureChangeQuadSize = ((*materialChangeIt).second - quadCounter);

				mRenderOperation.vertexData->vertexCount = textureChangeQuadSize * VERTICES_PER_QUAD;

				mRenderOperation.indexData->indexStart = indexOffset;
				mRenderOperation.indexData->indexCount = textureChangeQuadSize * 6;
				indexOffset  += mRenderOperation.indexData->indexCount;
			}

			// Render
			{
				// set render properties prior to rendering.
				_initRenderState();

				Ogre::SceneManager *scnMgr = mGUIManager->getViewport()->getCamera()->getSceneManager();
				
				Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName((*materialChangeIt).first);
				mat->load();
				mat->setLightingEnabled(false);
				Ogre::Technique *t = mat->getBestTechnique(0);

				Ogre::Technique::PassIterator passIt= t->getPassIterator();
				while (passIt.hasMoreElements())
				{		
					Ogre::Pass *pass = passIt.getNext();
					scnMgr->_setPass(pass);

					// Do we need to update GPU program parameters?
					if (pass->isProgrammable())
					{
						if (pass->hasVertexProgram())
						{
							mRenderSystem->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, 
								pass->getVertexProgramParameters());
						}
						if (pass->hasFragmentProgram())
						{
							mRenderSystem->bindGpuProgramParameters(Ogre::GPT_FRAGMENT_PROGRAM, 
								pass->getFragmentProgramParameters());
						}
					}
					
					// nfz: set up multipass rendering
					mRenderSystem->setCurrentPassIterationCount(pass->getPassIterationCount());
					mRenderSystem->_render(mRenderOperation);

				}
			}
			// store current Quad index Position to offset next render vertex/index Positions
			quadCounter = (*materialChangeIt).second;
		}

		mGUIManager->getViewport()->setShadowsEnabled(shadowsEnabled);
	}

	bool VertexBuffer::empty()
	{
		return (mVertexBufferUsage == 0);
	}

	bool VertexBuffer::full()
	{
		return (mVertexBufferSize == mVertexBufferUsage);
	}

	size_t VertexBuffer::getBufferSize()
	{
		return mVertexBufferSize;
	}

	bool VertexBuffer::getUpdateBeforeRender()
	{
		return mUpdateBeforeRender;
	}

	void VertexBuffer::render()
	{
		if(mUpdateBeforeRender) 
			update();
		_renderVertexBuffer();
	}

	void VertexBuffer::resizeVertexBuffer( const size_t numberOfVertices )
	{
		_destroyVertexBuffer();
		_destroyIndexBuffer();
		mVertexBufferSize = numberOfVertices;
		_createVertexBuffer();
		_createIndexBuffer();
	}

	void VertexBuffer::setData(QuadList* l)
	{
		mRenderObjectList = l;
	}

	void VertexBuffer::setUpdateBeforeRender(bool update)
	{
		mUpdateBeforeRender = update;
	}

	
	void VertexBuffer::update()
	{
		if((mRenderObjectList == NULL) || (mRenderObjectList->empty())) 
			return;
		
		// check if visible.
		{
			mVisibleRenderObjectList.clear();

			QuadList::iterator it;
			for( it = mRenderObjectList->begin(); it != mRenderObjectList->end(); ++it )
			{
				// skip all invisible RenderObjects
				if( (!(*it)->visible()) || (*it)->getMaterialName().empty() ) 
					continue;
				mVisibleRenderObjectList.push_back(*it);
			}
			// only updates if visible.
			if(mVisibleRenderObjectList.empty())
				return;
		}

		const size_t quadCount = mVisibleRenderObjectList.size();
		const size_t vertexCount = quadCount * VERTICES_PER_QUAD;
		mVertexBufferUsage = vertexCount;
		assert (vertexCount <= mVertexBufferSize);



		// batch per Texture.
		{
			// already sorted by zOrder and Texture, now grouping them in one batch per texture.
			mMaterialChangeList.clear();
			unsigned int quadCounter = 0;
			Ogre::String currentMaterial = mVisibleRenderObjectList.front()->getMaterialName();
			mMaterialChangeList.push_back( std::make_pair(currentMaterial,0) );
			for(QuadList::iterator it = mVisibleRenderObjectList.begin(); it != mVisibleRenderObjectList.end(); ++it)
			{
				// Every time a quad's texture is different than the previous quads, we record the quad's index.
				// This is useful for texture batching, and speeds up the _renderVertexBuffer function some.
				if((*it)->getMaterialName() != currentMaterial)
				{
					currentMaterial = (*it)->getMaterialName();
					mMaterialChangeList.back().second = quadCounter;
					mMaterialChangeList.push_back( std::make_pair(currentMaterial,0) );
				}

				++quadCounter;
			}
			// push one last value onto the list, so the remaining textures are taken into consideration, when list is used in _renderVertexBuffer
			mMaterialChangeList.back().second = quadCounter;
		}
		// Fill Vertex Buffer
		{
			// Note that locking with HBL_DISCARD will give us new, blank memory.
			Vertex *vertexBufferPtr = (Vertex*) mVertexBuffer->lock ( Ogre::HardwareVertexBuffer::HBL_DISCARD );
			QuadList::iterator it;
			for( it = mVisibleRenderObjectList.begin(); it != mVisibleRenderObjectList.end(); ++it )
			{
				// get pointer to beginning of array.
				Vertex *verts = (*it)->getVertices();
				for(size_t vertIndex = 0; vertIndex < VERTICES_PER_QUAD; ++vertIndex)
				{
					vertexBufferPtr[vertIndex].pos = verts->pos;
					vertexBufferPtr[vertIndex].color = verts->color;
					vertexBufferPtr[vertIndex].uv = verts->uv;

					// increment pointer through array.
					++verts;
				}
				vertexBufferPtr += VERTICES_PER_QUAD;
			}
			mVertexBuffer->unlock();
		}

		// Fill Index Buffer
		{
			// Handles 32bits indexes for really loud UI.
			if (mIndexBuffer->getType() == Ogre::HardwareIndexBuffer::IT_32BIT)
			{
				unsigned int* mIndexBufferPtr = (unsigned int*)mIndexBuffer->lock ( Ogre::HardwareVertexBuffer::HBL_DISCARD );
				unsigned int offset = 0;
				for (size_t k = 0; k < quadCount; k++)
				{
					*mIndexBufferPtr++ = offset + 0;
					*mIndexBufferPtr++ = offset + 1;
					*mIndexBufferPtr++ = offset + 3;

					*mIndexBufferPtr++ = offset + 1;
					*mIndexBufferPtr++ = offset + 2;
					*mIndexBufferPtr++ = offset + 3;

					offset += 4;
				}
				mIndexBuffer->unlock();
			}
			else
			{
				unsigned short* mIndexBufferPtr = (unsigned short*)mIndexBuffer->lock ( Ogre::HardwareVertexBuffer::HBL_DISCARD );
				unsigned short offset = 0;
				for (size_t k = 0; k < quadCount; k++)
				{
					*mIndexBufferPtr++ = offset + 0;
					*mIndexBufferPtr++ = offset + 1;
					*mIndexBufferPtr++ = offset + 3;

					*mIndexBufferPtr++ = offset + 1;
					*mIndexBufferPtr++ = offset + 2;
					*mIndexBufferPtr++ = offset + 3;

					offset += 4;
				}
				mIndexBuffer->unlock();
			}
		}
	}
}
