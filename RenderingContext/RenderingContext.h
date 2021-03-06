/*
	This file is part of the Rendering library.
	Copyright (C) 2007-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2007-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2007-2012 Ralf Petring <ralf@petring.net>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef RENDERING_RENDERINCONTEXT_H_
#define RENDERING_RENDERINCONTEXT_H_

#include <Util/CountedObjectWrapper.h>
#include <cstdint>
#include <memory>
#include <functional>

namespace Geometry {
template<typename _T> class _Matrix4x4;
typedef _Matrix4x4<float> Matrix4x4;
template<typename _T> class _Rect;
typedef _Rect<int> Rect_i;
}
namespace Util {
class Color4f;
class StringIdentifier;
}
namespace Rendering {
class AlphaTestParameters;
class BlendingParameters;
class BufferObject;
class ColorBufferParameters;
class CullFaceParameters;
class DepthBufferParameters;
class FBO;
class LightParameters;
class LightingParameters;
class LineParameters;
class MaterialParameters;
class Mesh;
class PointParameters;
class PolygonModeParameters;
class PolygonOffsetParameters;
class ScissorParameters;
class StencilParameters;
class Shader;
class Texture;
class Uniform;
class UniformRegistry;
class VertexAttribute;

typedef Util::CountedObjectWrapper<BufferObject> CountedBufferObject;

class RenderingContext {

	//! @name General
	//	@{
private:
	// Use Pimpl idiom
	class InternalState;
	std::unique_ptr<InternalState> state;

	bool immediate;

public:

	RenderingContext();
	~RenderingContext();

	//! has to return true iff normal display of mesh shall be executed
	typedef std::function<void (RenderingContext & rc,Mesh * mesh,uint32_t firstElement,uint32_t elementCount)> DisplayMeshFn;
private:
	DisplayMeshFn displayMeshFn;
public:
	void setDisplayMeshFn(DisplayMeshFn fn){ displayMeshFn = fn; };
	void resetDisplayMeshFn();

	void displayMesh(Mesh * mesh,uint32_t firstElement,uint32_t elementCount){ displayMeshFn(*this, mesh,firstElement,elementCount); }
	void displayMesh(Mesh * mesh);

	void setImmediateMode(const bool enabled);
	bool getImmediateMode() const {
		return immediate;
	}

	void applyChanges(bool forced = false);
	//	@}

	// -----------------------------------

	/*!	@name GL Helper */
	//	@{
	static void clearScreen(const Util::Color4f & color);
	static void initGLState();
	void clearScreenRect(const Geometry::Rect_i & rect, const Util::Color4f & color, bool clearDepth=true);

	/*! On AMD/ATI-cards, if a Shader accesses a non-existing vertex attribute (even in a branch that should not be
		executed), it seems that it accesses the data of the default GL_VERTEX_ARRAY attribute. If this is not set, the
		fragment's calculation fails and the object gets invisible. This workaround checks if an ATI/AMD card is used and
		in the MeshVertexData::bind()-function, the GL_VERTEX_ARRAY client state is enabled even if it is not required by
		the used Shader.
		\note The workaround is enabled if the GL_RENDERER-string contains 'AMD' or 'ATI'
	*/
	static bool useAMDAttrBugWorkaround();

	/**
	 * Flush the GL commands buffer.
	 * @see glFlush
	 */
	static void flush();

	/**
	 * Block until all GL commands are complete.
	 * @see glFinish
	 */
	static void finish();
	// @}

	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// Parameters (sorted alphabetically)

	//! @name AlphaTest
	//	@{
	const AlphaTestParameters & getAlphaTestParameters() const;
	void popAlphaTest();
	void pushAlphaTest();
	void pushAndSetAlphaTest(const AlphaTestParameters & alphaTestParameter);
	void setAlphaTest(const AlphaTestParameters & alphaTestParameter);
	// @}

	// ------

	//! @name Blending
	//	@{
	const BlendingParameters & getBlendingParameters() const;
	void popBlending();
	void pushBlending();
	void pushAndSetBlending(const BlendingParameters & blendingParameter);
	void setBlending(const BlendingParameters & blendingParameter);
	// @}

	// ------

	/*! @name Camera Matrix
	 camera matrix == inverse world matrix of camera node == default model view matrix	*/
	//	@{
	void setInverseCameraMatrix(const Geometry::Matrix4x4 & matrix);
	const Geometry::Matrix4x4 & getCameraMatrix() const;
	const Geometry::Matrix4x4 & getInverseCameraMatrix() const;
	// @}

	// ------

	//! @name ColorBuffer
	//	@{
	const ColorBufferParameters & getColorBufferParameters() const;
	void popColorBuffer();
	void pushColorBuffer();
	void pushAndSetColorBuffer(const ColorBufferParameters & colorBufferParameter);
	void setColorBuffer(const ColorBufferParameters & colorBufferParameter);

	/**
	 * Clear the color buffer.
	 *
	 * @param clearValue Color that the color buffer is filled with.
	 * The color components are clamped to [0, 1].
	 * @see glClearColor
	 * @see Parameter GL_COLOR_BUFFER_BIT of glClear
	 */
	void clearColor(const Util::Color4f & clearValue);
	// @}
	// ------

	//! @name CullFace
	//	@{
	const CullFaceParameters & getCullFaceParameters() const;
	void popCullFace();
	void pushCullFace();
	void pushAndSetCullFace(const CullFaceParameters & cullFaceParameters);
	void setCullFace(const CullFaceParameters & cullFaceParameters);
	// @}

	// ------

	//! @name DepthBuffer
	//	@{
	const DepthBufferParameters & getDepthBufferParameters() const;
	void popDepthBuffer();
	void pushDepthBuffer();
	void pushAndSetDepthBuffer(const DepthBufferParameters & depthBufferParameter);
	void setDepthBuffer(const DepthBufferParameters & depthBufferParameter);

	/**
	 * Clear the depth buffer.
	 *
	 * @param clearValue Value that the depth buffer is filled with
	 * The value is clamped to [0, 1].
	 * @see glClearDepth
	 * @see Parameter GL_DEPTH_BUFFER_BIT of glClear
	 */
	void clearDepth(float clearValue);
	// @}

	// ------

	//! @name FBO
	//	@{
	FBO * getActiveFBO() const;
	void popFBO();
	void pushFBO();
	void pushAndSetFBO(FBO * fbo);
	void setFBO(FBO * fbo);
	// @}

	// ------

	//! @name Global Uniforms
	//	@{
	void setGlobalUniform(const Uniform & u);
	const Uniform & getGlobalUniform(const Util::StringIdentifier & uniformName);
	// @}

	// ------

	//! @name Lighting
	//	@{
	const LightingParameters & getLightingParameters() const;
	void popLighting();
	void pushLighting();
	void pushAndSetLighting(const LightingParameters & lightingParameter);
	void setLighting(const LightingParameters & lightingParameter);

	// ------

	//! @name Lights
	//	@{
	/**
	 * Activate the light given by the parameters.
	 *
	 * @param light Parameters of a light source.
	 * @return Light number that was used for this light. This number has to be used to deactivate the light.
	 */
	uint8_t enableLight(const LightParameters & light);

	/**
	 * Deactivate a previuosly activated light.
	 *
	 * @param lightNumber Light number that was returned by @a enableLight.
	 */
	void disableLight(uint8_t lightNumber);
	// @}

	// ------

	//! @name Line
	//	@{
	const LineParameters & getLineParameters() const;
	void popLine();
	void pushLine();
	void pushAndSetLine(const LineParameters & lineParameters);
	void setLine(const LineParameters & lineParameters);
	// @}

	// ------

	//! @name Material
	//	@{
	//! Return the active material.
	const MaterialParameters & getMaterial() const;
	//! Pop a material from the top of the stack and activate it. Deactivate material usage if stack is empty.
	void popMaterial();
	//! Push the given material onto the material stack.
	void pushMaterial();
	//! Push the given material onto the material stack and activate it.
	void pushAndSetMaterial(const MaterialParameters & material);
	//! Convert the given color to a material, and call @a pushAndSetMaterial
	void pushAndSetColorMaterial(const Util::Color4f & color);
	//! Activate the given material.
	void setMaterial(const MaterialParameters & material);


	// @}

	// ------

	//! @name Model View Matrix
	//	@{
	//! resets the model view matrix to the default (camera matrix)
	void resetMatrix();
	const Geometry::Matrix4x4 & getMatrix() const;
	//! Save the current model view matrix onto the matrix stack.
	void pushMatrix();
	//! Multiply the given matrix @a matrix with the current model view matrix.
	void multMatrix(const Geometry::Matrix4x4 & matrix);
	//! Replace the current model view matrix with the given matrix @a matrix.
	void setMatrix(const Geometry::Matrix4x4 & matrix);
	//! Restore the current model view matrix from the matrix stack.
	void popMatrix();
	// @}
	
	// ------

	//! @name Point
	//	@{
	const PointParameters & getPointParameters() const;
	void popPointParameters();
	void pushPointParameters();
	void pushAndSetPointParameters(const PointParameters & pointParameters);
	void setPointParameters(const PointParameters & pointParameters);
	// @}
	// ------

	//! @name PolygonMode
	//	@{
	const PolygonModeParameters & getPolygonModeParameters() const;
	void popPolygonMode();
	void pushPolygonMode();
	void pushAndSetPolygonMode(const PolygonModeParameters & polygonModeParameter);
	void setPolygonMode(const PolygonModeParameters & polygonModeParameter);
	// @}

	// ------

	//! @name PolygonOffset
	//	@{
	const PolygonOffsetParameters & getPolygonOffsetParameters() const;
	void popPolygonOffset();
	void pushPolygonOffset();
	void pushAndSetPolygonOffset(const PolygonOffsetParameters & polygonOffsetParameter);
	void setPolygonOffset(const PolygonOffsetParameters & polygonOffsetParameter);
	// @}

	// ------

	//! @name Projection Matrix
	//	@{
	void pushProjectionMatrix();
	void popProjectionMatrix();
	void setProjectionMatrix(const Geometry::Matrix4x4 & matrix);
	const Geometry::Matrix4x4 & getProjectionMatrix() const;
	// @}

	// ------

	//! @name Shader
	//	@{
	void pushAndSetShader(Shader * shader);
	void pushShader();
	void popShader();
	bool isShaderEnabled(Shader * shader);
	Shader * getActiveShader();
	const Shader * getActiveShader() const;
	void setShader(Shader * shader); // shader may be nullptr

	//! (internal) called by Shader::setUniform(...)
	void _setUniformOnShader(Shader * shader, const Uniform & uniform, bool warnIfUnused, bool forced);

	// @}

	// ------

	//! @name Scissor
	//	@{
	const ScissorParameters & getScissor() const;
	void popScissor();
	void pushScissor();
	void pushAndSetScissor(const ScissorParameters & scissorParameters);
	void setScissor(const ScissorParameters & scissorParameters);
	// @}

// ------

	//! @name Stencil
	//	@{
	const StencilParameters & getStencilParamters() const;
	void popStencil();
	void pushStencil();
	void pushAndSetStencil(const StencilParameters & stencilParameter);
	void setStencil(const StencilParameters & stencilParameter);

	/**
	 * Clear the stencil buffer.
	 *
	 * @param clearValue Value that the stencil buffer is filled with
	 * @see glClearStencil
	 * @see Parameter GL_STENCIL_BUFFER_BIT of glClear
	 */
	void clearStencil(int32_t clearValue);
	// @}

	// ------

	/*! @name Textures
	 \todo Move array of activeTextures to RenderingData to allow delayed binding
	 */
	//	@{
	Texture * getTexture(uint32_t unit);
	void pushTexture(uint32_t unit);
	void pushAndSetTexture(uint32_t unit, Texture * texture);
	void popTexture(uint32_t unit);

	//! \note texture may be nullptr
	void setTexture(uint32_t unit, Texture * texture);
	// @}
	
	// ------

	//! @name Transform Feedback
	//	@{
	static bool isTransformFeedbackSupported();
	static bool requestTransformFeedbackSupport(); //! like isTransformFeedbackSupported(), but once issues a warning on failure.
	
	CountedBufferObject * getActiveTransformFeedbackBuffer() const;
	void popTransformFeedbackBufferStatus();
	void pushTransformFeedbackBufferStatus();
	void setTransformFeedbackBuffer(CountedBufferObject * buffer);
	void _startTransformFeedback(uint32_t);
	void startTransformFeedback_lines();
	void startTransformFeedback_points();
	void startTransformFeedback_triangles();
	void stopTransformFeedback();
	// @}

	// ------

	//! @name VBO Client States
	// @{
	//! Activate the given client state.
	void enableClientState(uint32_t clientState);

	//! Deactivate all client states that were activated before.
	void disableAllClientStates();

	//! Activate the texture coordinate client state for the given texture unit.
	void enableTextureClientState(uint32_t textureUnit);

	//! Deactivate the texture coordinate client states for all texture units that were activated before.
	void disableAllTextureClientStates();

	/**
	 * Bind a vertex attribute to a variable inside a shader program.
	 *
	 * @param attr Attribute description (including variable name)
	 * @param data Pointer to the vertex data (or @c nullptr if a buffer object is active)
	 * @param stride Size of a vertex in bytes
	 */
	void enableVertexAttribArray(const VertexAttribute & attr, const uint8_t * data, int32_t stride);

	//! Disable all vertex attribute array.
	void disableAllVertexAttribArrays();
	// @}

	// ------

	//! @name Viewport and window's size
	// @{
	/**
	 * Get the OpenGL window's client area.
	 * In almost all cases, the position will be (0, 0).
	 * The width and height differs with the size of the window.
	 * @note This value has to be set manually by calling setWindowClientArea() after creating the RenderingContext
	 */
	const Geometry::Rect_i & getWindowClientArea() const;

	//! Read the current viewport.
	const Geometry::Rect_i & getViewport() const;
	//! Restore the viewport from the top of the viewport stack.
	void popViewport();

	//! Save the current viewport onto the viewport stack.
	void pushViewport();

	//! Set the current viewport.
	void setViewport(const Geometry::Rect_i & viewport);

	void setWindowClientArea(const Geometry::Rect_i & clientArea);
	// @}
};

}

#endif /* RENDERING_RENDERINCONTEXT_H_ */
