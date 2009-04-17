/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *	  $Id$
 *
 *****************************************************************************/

// This source code is a fixed and maintained version of the Null render system written by xyzzy@ogre3d.org forum

#include "stdafx.h"
#include "NullRenderSystem.h"
#include "NullHardwareBufferManager.h"
#include "NullTextureManager.h"
#include "NullRenderWindow.h"

namespace Ogre {
	
	//---------------------------------------------------------------------
	NULLRenderSystem::NULLRenderSystem() {
		m_pHardwareBufferManager = new NULLHardwareBufferManager();
	}
	//---------------------------------------------------------------------
	NULLRenderSystem::~NULLRenderSystem() {
		delete m_pHardwareBufferManager;
	
		shutdown();
	}
	//---------------------------------------------------------------------
	const String& NULLRenderSystem::getName() const {
		static String strName("NULL Rendering Subsystem");
		return strName;
	}
	//---------------------------------------------------------------------
	
	void NULLRenderSystem::setConfigOption(const String &name, const String &value) {
	
	}
	//---------------------------------------------------------------------
	String NULLRenderSystem::validateConfigOptions() {
		return StringUtil::BLANK;
	}
	//---------------------------------------------------------------------
	ConfigOptionMap& NULLRenderSystem::getConfigOptions() {
		return m_options;
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::reinitialise() {
		this->shutdown();
		this->_initialise(true);
	}
	//---------------------------------------------------------------------
	RenderWindow* NULLRenderSystem::_createRenderWindow(const String &name,
			unsigned int width, unsigned int height, bool fullScreen,
			const NameValuePairList *miscParams) {
	
		mTextureManager = new NULLTextureManager();
		mGpuProgramManager = new NULLGpuProgramManager();
	
		RenderWindow* win = new NULLRenderWindow();
	
		win->create(name, width, height, fullScreen, miscParams);
	
		attachRenderTarget(*win);
		return win;
	}
	//-----------------------------------------------------------------------
	MultiRenderTarget * NULLRenderSystem::createMultiRenderTarget(
			const String & name) {
		return NULL;
	}
	//---------------------------------------------------------------------
	String NULLRenderSystem::getErrorDescription(long errorNumber) const {
		const String errMsg = "Not implemented";
		return errMsg;
	}
	//---------------------------------------------------------------------
	VertexElementType NULLRenderSystem::getColourVertexElementType(void) const {
		return VET_COLOUR_ARGB;
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_convertProjectionMatrix(const Matrix4& matrix,
			Matrix4& dest, bool forGpuProgram) {
		dest = matrix;
	
		// Convert depth range from [-1,+1] to [0,1]
		dest[2][0] = (dest[2][0] + dest[3][0]) / 2;
		dest[2][1] = (dest[2][1] + dest[3][1]) / 2;
		dest[2][2] = (dest[2][2] + dest[3][2]) / 2;
		dest[2][3] = (dest[2][3] + dest[3][3]) / 2;
	
		if (!forGpuProgram) {
			// Convert right-handed to left-handed
			dest[0][2] = -dest[0][2];
			dest[1][2] = -dest[1][2];
			dest[2][2] = -dest[2][2];
			dest[3][2] = -dest[3][2];
		}
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_makeProjectionMatrix(const Radian& fovy, Real aspect,
			Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_makeOrthoMatrix(const Radian& fovy, Real aspect,
			Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setAmbientLight(float r, float g, float b) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_useLights(const LightList& lights, unsigned short limit) {
	
	}
	
	//---------------------------------------------------------------------
	void NULLRenderSystem::setShadingType(ShadeOptions so) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setLightingEnabled(bool enabled) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setViewMatrix(const Matrix4 &m) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setProjectionMatrix(const Matrix4 &m) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setWorldMatrix(const Matrix4 &m) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setSurfaceParams(const ColourValue &ambient,
			const ColourValue &diffuse, const ColourValue &specular,
			const ColourValue &emissive, Real shininess,
			TrackVertexColourType tracking) {
	
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setPointParameters(Real size, bool attenuationEnabled,
			Real constant, Real linear, Real quadratic, Real minSize, Real maxSize) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setPointSpritesEnabled(bool enabled) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTexture(size_t stage, bool enabled,
			const TexturePtr& tex) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureCoordSet(size_t stage, size_t index) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureCoordCalculation(size_t stage,
			TexCoordCalcMethod m, const Frustum* frustum) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureMipmapBias(size_t unit, float bias) {
		
	}
	
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureMatrix(size_t stage, const Matrix4& xForm) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureAddressingMode(size_t stage,
			const TextureUnitState::UVWAddressingMode& uvw) {
	}
	//-----------------------------------------------------------------------------
	void NULLRenderSystem::_setTextureBorderColour(size_t stage,
			const ColourValue& colour) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureBlendMode(size_t stage,
			const LayerBlendModeEx& bm) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setSceneBlending(SceneBlendFactor sourceFactor,
			SceneBlendFactor destFactor) {
	}
	
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setSeparateSceneBlending(SceneBlendFactor sourceFactor, 
			SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, 
			SceneBlendFactor destFactorAlpha) {
		
	}
	
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setAlphaRejectSettings(CompareFunction func,
			unsigned char value, bool alphaToCoverage) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setCullingMode(CullingMode mode) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setDepthBufferParams(bool depthTest, bool depthWrite,
			CompareFunction depthFunction) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setDepthBufferCheckEnabled(bool enabled) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setDepthBufferWriteEnabled(bool enabled) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setDepthBufferFunction(CompareFunction func) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setDepthBias(float bias, float slopeScaleBias) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setColourBufferWriteEnabled(bool red, bool green,
			bool blue, bool alpha) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setFog(FogMode mode, const ColourValue& colour,
			Real densitiy, Real start, Real end) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setPolygonMode(PolygonMode level) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setStencilCheckEnabled(bool enabled) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setStencilBufferParams(CompareFunction func,
			uint32 refValue, uint32 mask, StencilOperation stencilFailOp,
			StencilOperation depthFailOp, StencilOperation passOp,
			bool twoSidedOperation) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureUnitFiltering(size_t unit, FilterType ftype,
			FilterOptions filter) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setTextureLayerAnisotropy(size_t unit,
			unsigned int maxAnisotropy) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_setViewport(Viewport *vp) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_beginFrame() {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_endFrame() {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setVertexDeclaration(VertexDeclaration* decl) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setVertexBufferBinding(VertexBufferBinding* binding) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_render(const RenderOperation& op) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setNormaliseNormals(bool normalise) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::bindGpuProgram(GpuProgram* prg) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::unbindGpuProgram(GpuProgramType gptype) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::bindGpuProgramParameters(GpuProgramType gptype,
			GpuProgramParametersSharedPtr params) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::bindGpuProgramPassIterationParameters(
			GpuProgramType gptype) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::setScissorTest(bool enabled, size_t left, size_t top,
			size_t right, size_t bottom) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::clearFrameBuffer(unsigned int buffers,
			const ColourValue& colour, Real depth, unsigned short stencil) {
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_makeProjectionMatrix(Real left, Real right,
			Real bottom, Real top, Real nearPlane, Real farPlane, Matrix4& dest,
			bool forGpuProgram) {
	}
	
	// ------------------------------------------------------------------
	void NULLRenderSystem::setClipPlane(ushort index, Real A, Real B, Real C,
			Real D) {
	}
	
	// ------------------------------------------------------------------
	void NULLRenderSystem::enableClipPlane(ushort index, bool enable) {
	}
	//---------------------------------------------------------------------
	HardwareOcclusionQuery* NULLRenderSystem::createHardwareOcclusionQuery(void) {
		return NULL;
	}
	//---------------------------------------------------------------------
	Real NULLRenderSystem::getHorizontalTexelOffset(void) {
		return -0.5f;
	}
	//---------------------------------------------------------------------
	Real NULLRenderSystem::getVerticalTexelOffset(void) {
		return -0.5f;
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::_applyObliqueDepthProjection(Matrix4& matrix,
			const Plane& plane, bool forGpuProgram) {
	}
	//---------------------------------------------------------------------
	Real NULLRenderSystem::getMinimumDepthInputValue(void) {
		return 0.0f;
	}
	
	//---------------------------------------------------------------------
	Real NULLRenderSystem::getMaximumDepthInputValue(void) {
		return -1.0f;
	}
	
	//---------------------------------------------------------------------
	void NULLRenderSystem::preExtraThreadsStarted() {
		
	}
	//---------------------------------------------------------------------
	void NULLRenderSystem::postExtraThreadsStarted() {
		
	}
	//---------------------------------------------------------------------	
	void NULLRenderSystem::registerThread() {
		
	}
	//---------------------------------------------------------------------	
	void NULLRenderSystem::unregisterThread() {
		
	}
	
	//---------------------------------------------------------------------
	RenderSystemCapabilities* NULLRenderSystem::createRenderSystemCapabilities() const {
		mRealCapabilities->setCapability(RSC_HWSTENCIL);
		mRealCapabilities->setStencilBufferBitDepth(8);
		mRealCapabilities->setNumTextureUnits(16);
		mRealCapabilities->setCapability(RSC_ANISOTROPY);
		mRealCapabilities->setCapability(RSC_AUTOMIPMAP);
		mRealCapabilities->setCapability(RSC_BLENDING);
		mRealCapabilities->setCapability(RSC_DOT3);
		mRealCapabilities->setCapability(RSC_CUBEMAPPING);
		mRealCapabilities->setCapability(RSC_TEXTURE_COMPRESSION);
		mRealCapabilities->setCapability(RSC_TEXTURE_COMPRESSION_DXT);
		mRealCapabilities->setCapability(RSC_VBO);
		mRealCapabilities->setCapability(RSC_SCISSOR_TEST);
		mRealCapabilities->setCapability(RSC_TWO_SIDED_STENCIL);
		mRealCapabilities->setCapability(RSC_STENCIL_WRAP);
		mRealCapabilities->setCapability(RSC_HWOCCLUSION);
		mRealCapabilities->setCapability(RSC_USER_CLIP_PLANES);
		mRealCapabilities->setCapability(RSC_VERTEX_FORMAT_UBYTE4);
		mRealCapabilities->setCapability(RSC_INFINITE_FAR_PLANE);
		mRealCapabilities->setCapability(RSC_TEXTURE_3D);
		mRealCapabilities->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
		mRealCapabilities->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
		mRealCapabilities->setNonPOW2TexturesLimited(true);
		mRealCapabilities->setCapability(RSC_HWRENDER_TO_TEXTURE);
		mRealCapabilities->setCapability(RSC_TEXTURE_FLOAT);
		mRealCapabilities->setCapability(RSC_POINT_SPRITES);
		mRealCapabilities->setCapability(RSC_POINT_EXTENDED_PARAMETERS);
		mRealCapabilities->setMaxPointSize(256);

		return mRealCapabilities;
	}
	
	//---------------------------------------------------------------------
	void NULLRenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary) {
		mRealCapabilities = caps;
	}
	
	RenderWindow* NULLRenderSystem::_initialise(bool autoCreateWindow,
			const String& windowTitle) {
		RenderWindow* autoWindow = NULL;
	
		// Init using current settings
		if (autoCreateWindow) {
			autoWindow = this->_createRenderWindow(windowTitle, 640, 480, false,
					NULL);
		}
	
		// call superclass method
		RenderSystem::_initialise(autoCreateWindow);
	
		return autoWindow;
	}

}

