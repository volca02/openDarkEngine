#ifndef __NULLRENDERSYSTEM_H
#define __NULLRENDERSYSTEM_H

#include "stdafx.h"
#include "NullHardwareBufferManager.h"
#include "NullGpuProgramManager.h"

namespace Ogre {

class NULLRenderSystem: public RenderSystem {
public:
	NULLRenderSystem();
	~NULLRenderSystem();

	ConfigOptionMap m_options;

	virtual const String& getName(void) const;
	virtual ConfigOptionMap& getConfigOptions(void);
	virtual void setConfigOption(const String &name, const String &value);
	virtual HardwareOcclusionQuery* createHardwareOcclusionQuery(void);

	virtual String validateConfigOptions(void);

	virtual void reinitialise(void);

	virtual void setAmbientLight(float r, float g, float b);

	virtual void setShadingType(ShadeOptions so);

	virtual void setLightingEnabled(bool enabled);

	virtual MultiRenderTarget * createMultiRenderTarget(const String & name);

	virtual String getErrorDescription(long errorNumber) const;

	// ------------------------------------------------------------------------
	//                     Internal Rendering Access
	// All methods below here are normally only called by other OGRE classes
	// They can be called by library user if required
	// ------------------------------------------------------------------------


	virtual void _useLights(const LightList& lights, unsigned short limit);
	virtual void _setWorldMatrix(const Matrix4 &m);
	virtual void _setViewMatrix(const Matrix4 &m);
	virtual void _setProjectionMatrix(const Matrix4 &m);
	virtual void _setSurfaceParams(const ColourValue &ambient,
			const ColourValue &diffuse, const ColourValue &specular,
			const ColourValue &emissive, Real shininess,
			TrackVertexColourType tracking = TVC_NONE);

	virtual void _setPointSpritesEnabled(bool enabled);
	virtual void _setPointParameters(Real size, bool attenuationEnabled,
			Real constant, Real linear, Real quadratic, Real minSize,
			Real maxSize);

	virtual void _setTexture(size_t unit, bool enabled, const TexturePtr& tex);

	virtual void _setTextureCoordSet(size_t unit, size_t index);

	virtual void _setTextureCoordCalculation(size_t unit, TexCoordCalcMethod m,
			const Frustum* frustum = 0);

	virtual void _setTextureBlendMode(size_t unit, const LayerBlendModeEx& bm);

	virtual void _setTextureMipmapBias(size_t unit, float bias);
	
	virtual void _setTextureUnitFiltering(size_t unit, FilterType ftype,
			FilterOptions filter);

	virtual void _setTextureLayerAnisotropy(size_t unit,
			unsigned int maxAnisotropy);

	virtual void _setTextureAddressingMode(size_t unit,
			const TextureUnitState::UVWAddressingMode& uvw);

	virtual void
			_setTextureBorderColour(size_t unit, const ColourValue& colour);

	virtual void _setTextureMatrix(size_t unit, const Matrix4& xform);

	virtual void _setSceneBlending(SceneBlendFactor sourceFactor,
			SceneBlendFactor destFactor);

	virtual void _setSeparateSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha);
	
	virtual void _setAlphaRejectSettings(CompareFunction func,
			unsigned char value, bool alphaToCoverage);
	virtual void _beginFrame(void);

	virtual void _endFrame(void);
	virtual void _setViewport(Viewport *vp);
	virtual void _setCullingMode(CullingMode mode);

	virtual void _setDepthBufferParams(bool depthTest = true, bool depthWrite =
			true, CompareFunction depthFunction = CMPF_LESS_EQUAL);
	virtual void _setDepthBufferCheckEnabled(bool enabled = true);
	virtual void _setDepthBufferWriteEnabled(bool enabled = true);
	virtual void
			_setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL);
	virtual void _setColourBufferWriteEnabled(bool red, bool green, bool blue,
			bool alpha);
	virtual void _setDepthBias(float bias, float slopeScaleBias);
	virtual void _setFog(FogMode mode = FOG_NONE, const ColourValue& colour =
			ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0,
			Real linearEnd = 1.0);

	virtual VertexElementType getColourVertexElementType(void) const;
	virtual void _convertProjectionMatrix(const Matrix4& matrix, Matrix4& dest,
			bool forGpuProgram = false);

	virtual void _makeProjectionMatrix(const Radian& fovy, Real aspect,
			Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram =
					false);

	virtual void _makeProjectionMatrix(Real left, Real right, Real bottom,
			Real top, Real nearPlane, Real farPlane, Matrix4& dest,
			bool forGpuProgram = false);
	virtual void _makeOrthoMatrix(const Radian& fovy, Real aspect,
			Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram =
					false);

	virtual void _applyObliqueDepthProjection(Matrix4& matrix,
			const Plane& plane, bool forGpuProgram);

	virtual void _setPolygonMode(PolygonMode level);

	virtual void setStencilCheckEnabled(bool enabled);
	virtual void setStencilBufferParams(
			CompareFunction func = CMPF_ALWAYS_PASS, uint32 refValue = 0,
			uint32 mask = 0xFFFFFFFF,
			StencilOperation stencilFailOp = SOP_KEEP,
			StencilOperation depthFailOp = SOP_KEEP, StencilOperation passOp =
					SOP_KEEP, bool twoSidedOperation = false);

	virtual void setVertexDeclaration(VertexDeclaration* decl);
	virtual void setVertexBufferBinding(VertexBufferBinding* binding);
	virtual void setNormaliseNormals(bool normalise);

	virtual void _render(const RenderOperation& op);
	const RenderSystemCapabilities* getCapabilities(void) const {
		return mRealCapabilities;
	}
	virtual void bindGpuProgram(GpuProgram* prg);
	virtual void bindGpuProgramParameters(GpuProgramType gptype,
			GpuProgramParametersSharedPtr params);
	virtual void bindGpuProgramPassIterationParameters(GpuProgramType gptype);
	virtual void unbindGpuProgram(GpuProgramType gptype);

	virtual void setClipPlanesImpl(const PlaneList& clipPlanes);
	virtual void enableClipPlane(Ogre::ushort index, bool enable);
	virtual void setScissorTest(bool enabled, size_t left = 0, size_t top = 0,
			size_t right = 800, size_t bottom = 600);
	virtual void clearFrameBuffer(unsigned int buffers,
			const ColourValue& colour = ColourValue::Black, Real depth = 1.0f,
			unsigned short stencil = 0);
	virtual Real getHorizontalTexelOffset(void);
	virtual Real getVerticalTexelOffset(void);
	virtual Real getMinimumDepthInputValue(void);
	virtual Real getMaximumDepthInputValue(void);
	
	virtual void preExtraThreadsStarted();
	virtual void postExtraThreadsStarted();
	virtual void registerThread();
	virtual void unregisterThread();
	
	void setCurrentPassIterationCount(const size_t count) {
		mCurrentPassIterationCount = count;
	}
	
	virtual RenderSystemCapabilities* createRenderSystemCapabilities() const;
	
	virtual void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary);

	class _OgreExport Listener {
	public:
		Listener() {
		}
		virtual ~Listener() {
		}

		virtual void eventOccurred(const String& eventName,
				const NameValuePairList* parameters = 0);
	};
	virtual const StringVector& getRenderSystemEvents(void) const {
		return mEventNames;
	}
	void setClipPlane(Ogre::ushort index, Real A, Real B, Real C, Real D);

	virtual RenderWindow* _initialise(bool autoCreateWindow,
			const String& windowTitle = "null render window");
	
	RenderWindow* _createRenderWindow(const String &name,
			unsigned int width, unsigned int height, bool fullScreen,
			const NameValuePairList *miscParams = 0);
	
	NULLHardwareBufferManager * m_pHardwareBufferManager;
	NULLGpuProgramManager* mGpuProgramManager;
};

}
#endif
