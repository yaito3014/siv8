//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <array>
# include <memory>
# include <unordered_map>
# include <Siv3D/Renderer2D/IRenderer2D.hpp>
# include <Siv3D/Renderer2D/Vertex2DBuilder.hpp>
# include <Siv3D/Renderer2D/Vertex2DBufferPointer.hpp>
# include <Siv3D/Pattern/PatternParameters.hpp>
# include <Siv3D/LineStyle.hpp>
# include <Siv3D/FloatQuad.hpp>
# include <Siv3D/Array.hpp>
# include <Siv3D/Mat3x2.hpp>
# include <Siv3D/Mat3x3.hpp>
# include <Siv3D/ConstantBuffer.hpp>
# include <Siv3D/VertexShader.hpp>
# include <Siv3D/PixelShader.hpp>
# include <Siv3D/Renderer2D/Renderer2DCommon.hpp>
# include <Siv3D/Texture.hpp>
# include <Siv3D/BlendState.hpp>
# include <Siv3D/RasterizerState.hpp>
# include <Siv3D/SamplerState.hpp>
# include <Siv3D/Renderer2D/ConstantBuffer2DCommands.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace s3d
{
	// A complete 2D renderer. Geometry is tessellated by the common Vertex2DBuilder;
	// shaders + constants flow through the engine shader registry (CEngineShader_GL4)
	// and real UBOs, bound via a GL program pipeline (CShader_GL4) — matching the
	// D3D11/Metal backends. Supports solid/gradient shapes, textures/sprites, MSDF
	// text (incl. SDF outline/shadow/glow), all six fill patterns (every shape),
	// shadows, dashed/dotted lines, quad-warp, custom user shaders, and the full 2D
	// render state (blend/rasterizer/sampler/scissor/viewport). No-op TODO(linux):
	// render-to-texture / scene-letterbox (no scene FBO yet; MSAA lives on the window),
	// custom shader constant buffers, and the Halftone/Wave/Ripple/Weave/Truchet
	// pattern shaders (drawn with the solid shape shader until ported to GLSL).
	class CRenderer2D_GL4 final : public ISiv3DRenderer2D
	{
	public:

		~CRenderer2D_GL4() override;

		void init() override;

		void flush() override;

		////////////////////////////////////////////////////////////////
		//	solid-color shapes (tessellated by the common Vertex2DBuilder)
		////////////////////////////////////////////////////////////////

		void addLine(LineCap startCap, LineCap endCap, const Float2& start, const Float2& end, float thickness, const Float4(&colors)[2]) override
		{
			discard(Vertex2DBuilder::BuildLine(bufferCreator(), startCap, endCap, start, end, thickness, colors, getRMSScaling()));
		}
		void addLine(const LineStyle& style, const Float2& start, const Float2& end, float thickness, const Float4(&colors)[2]) override
		{
			const auto indexCount = Vertex2DBuilder::BuildLine(bufferCreator(), style, start, end, thickness, colors, getRMSScaling());

			if (style.type == LinePattern::Solid)
			{
				pushCommand(indexCount, Program::Shape, 0);
			}
			else
			{
				pushCommand(indexCount, Program::Line, 0, static_cast<uint8>(FromEnum(style.type)));
			}
		}
		void addArrow(LineCap startCap, const Float2& start, const Float2& end, float thickness, const Float2& headSize, const Float4(&colors)[2]) override
		{
			discard(Vertex2DBuilder::BuildArrow(bufferCreator(), startCap, start, end, thickness, headSize, colors, getRMSScaling()));
		}
		void addTriangle(const Float2(&points)[3], const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildTriangle(bufferCreator(), points, color));
		}
		void addTriangle(const Float2(&points)[3], const Float4(&colors)[3]) override
		{
			discard(Vertex2DBuilder::BuildTriangle(bufferCreator(), points, colors));
		}
		void addRect(const FloatRect& rect, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildRect(bufferCreator(), rect, color));
		}
		void addRect(const FloatRect& rect, const Float4(&colors)[4]) override
		{
			discard(Vertex2DBuilder::BuildRect(bufferCreator(), rect, colors));
		}
		void addRectFrame(const FloatRect& innerRect, float thickness, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildRectFrame(bufferCreator(), innerRect, thickness, colorType, color0, color1));
		}
		void addRectDashedFrame(const FloatRect& innerRect, float offset, float thickness, float dashRatio, uint32 dashCount, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildRectDashedFrame(bufferCreator(), innerRect, offset, thickness, dashRatio, dashCount, color));
		}
		void addCircle(const Float2& center, float r, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildCircle(bufferCreator(), center, r, colorType, color0, color1, getRMSScaling()));
		}
		void addCircleFrame(const Float2& center, float rInner, float thickness, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildCircleFrame(bufferCreator(), center, rInner, thickness, innerColor, outerColor, getRMSScaling()));
		}
		void addCircleDashedFrame(const Float2& center, float rInner, float startAngle, float thickness, float dashRatio, uint32 dashCount, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildCircleDashedFrame(bufferCreator(), center, rInner, startAngle, thickness, dashRatio, dashCount, innerColor, outerColor, getRMSScaling()));
		}
		void addCirclePie(const Float2& center, float r, float startAngle, float angle, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildCirclePie(bufferCreator(), center, r, startAngle, angle, innerColor, outerColor, getRMSScaling()));
		}
		void addCircleArc(LineCap lineCap, const Float2& center, float rInner, float startAngle, float angle, float thickness, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildCircleArc(bufferCreator(), lineCap, center, rInner, startAngle, angle, thickness, colorType, color0, color1, getRMSScaling()));
		}
		void addCircleSegment(const Float2& center, float r, float startAngle, float angle, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildCircleSegment(bufferCreator(), center, r, startAngle, angle, color, getRMSScaling()));
		}
		void addEllipse(const Float2& center, float a, float b, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildEllipse(bufferCreator(), center, a, b, colorType, color0, color1, getRMSScaling()));
		}
		void addEllipseFrame(const Float2& center, float a, float b, float innerThickness, float outerThickness, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildEllipseFrame(bufferCreator(), center, a, b, innerThickness, outerThickness, innerColor, outerColor, getRMSScaling()));
		}
		void addEllipseDashedFrame(const Float2& center, float a, float b, float innerThickness, float outerThickness, float offset, float dashRatio, uint32 dashCount, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildEllipseDashedFrame(bufferCreator(), center, a, b, innerThickness, outerThickness, offset, dashRatio, dashCount, innerColor, outerColor, getRMSScaling()));
		}
		void addEllipsePie(const Float2& center, float rx, float ry, float startAngle, float angle, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildEllipsePie(bufferCreator(), center, rx, ry, startAngle, angle, innerColor, outerColor, getRMSScaling()));
		}
		void addSuperEllipse(const Float2& center, float a, float b, float n, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildSuperEllipse(bufferCreator(), center, a, b, n, colorType, color0, color1, getRMSScaling()));
		}
		void addQuad(const FloatQuad& quad, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildQuad(bufferCreator(), quad, color));
		}
		void addQuad(const FloatQuad& quad, const Float4(&colors)[4]) override
		{
			discard(Vertex2DBuilder::BuildQuad(bufferCreator(), quad, colors));
		}
		void addRoundRect(const FloatRect& rect, float r, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildRoundRect(bufferCreator(), rect, r, color, getRMSScaling()));
		}
		void addRoundRect(const FloatRect& rect, float r, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildRoundRect(bufferCreator(), rect, r, colorType, color0, color1, getRMSScaling()));
		}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildRoundRectFrame(bufferCreator(), innerRect, innerR, outerRect, outerR, color, getRMSScaling()));
		}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildRoundRectFrame(bufferCreator(), innerRect, innerR, outerRect, outerR, colorType, color0, color1, getRMSScaling()));
		}
		void addRoundRectDashedFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, float offset, float dashRatio, uint32 dashCount, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildRoundRectDashedFrame(bufferCreator(), innerRect, innerR, outerRect, outerR, offset, dashRatio, dashCount, color, getRMSScaling()));
		}
		void addPolygon(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, const Optional<Float2>& offset, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildPolygon(bufferCreator(), vertices, triangleIndices, offset, color));
		}
		void addPolygon(std::span<const Float2> vertices, std::span<const Vertex2D::IndexType> indices, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildPolygon(bufferCreator(), vertices, indices, color));
		}
		void addPolygonTransformed(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, float s, float c, const Float2& offset, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildPolygonTransformed(bufferCreator(), vertices, triangleIndices, s, c, offset, color));
		}
		void addShape2DFrame(std::span<const Float2> vertices, const Optional<Float2>& offset, float thickness, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildShape2DFrame(bufferCreator(), vertices, offset, thickness, color, getRMSScaling()));
		}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildLineString(bufferCreator(), startCap, endCap, points, offset, thickness, inner, closeRing, color, getRMSScaling()));
		}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, const Float4& colorStart, const Float4& colorEnd) override
		{
			discard(Vertex2DBuilder::BuildLineString(bufferCreator(), startCap, endCap, points, offset, thickness, inner, colorStart, colorEnd, getRMSScaling()));
		}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, std::span<const ColorF> colors) override
		{
			discard(Vertex2DBuilder::BuildLineString(bufferCreator(), startCap, endCap, points, offset, thickness, inner, closeRing, colors, getRMSScaling()));
		}

		////////////////////////////////////////////////////////////////
		//	render state
		////////////////////////////////////////////////////////////////

		Float4 getColorMul() const override { return m_colorMul; }
		void setColorMul(const Float4& color) override { m_colorMul = color; }
		Float3 getColorAdd() const override { return m_colorAdd; }
		void setColorAdd(const Float3& color) override { m_colorAdd = color; }
		float getRMSScaling() const noexcept override { return m_rmsScaling; }
		const Mat3x2& getLocalTransform() const override { return m_localTransform; }
		void setLocalTransform(const Mat3x2& matrix) override { m_localTransform = matrix; updateRMSScaling(); }
		const Mat3x2& getCameraTransform() const override { return m_cameraTransform; }
		void setCameraTransform(const Mat3x2& matrix) override { m_cameraTransform = matrix; updateRMSScaling(); }
		const Mat3x3& getQuadWarpTransform() const override { return m_quadWarpTransform; }
		void setQuadWarpTransform(const Mat3x3& matrix) override { m_quadWarpTransform = matrix; }

		////////////////////////////////////////////////////////////////
		//	Pattern fills, textured shapes, shadows, quad-warp, SDF/custom shaders,
		//	and the full render state are all implemented; flush() binds the engine
		//	(or custom) shaders via the program pipeline and feeds them through UBOs.
		////////////////////////////////////////////////////////////////

		// pattern fills: the pattern program shades from the drawing-space position
		// (passed by the Pattern2D VS), so every shape just tessellates its geometry
		// (filled with the primary color) and routes through pushPatternCommand.
		void addTriangle(const Float2(&points)[3], const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildTriangle(bufferCreator(), points, pattern.primaryColor), pattern);
		}
		void addRect(const FloatRect& rect, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildRect(bufferCreator(), rect, pattern.primaryColor), pattern);
		}
		void addRectFrame(const FloatRect& innerRect, float thickness, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildRectFrame(bufferCreator(), innerRect, thickness, ColorFillDirection::InOut, pattern.primaryColor, pattern.primaryColor), pattern);
		}
		void addCircle(const Float2& center, float r, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildCircle(bufferCreator(), center, r, ColorFillDirection::InOut, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addCircleFrame(const Float2& center, float rInner, float thickness, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildCircleFrame(bufferCreator(), center, rInner, thickness, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addCirclePie(const Float2& center, float r, float startAngle, float angle, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildCirclePie(bufferCreator(), center, r, startAngle, angle, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addCircleArc(LineCap lineCap, const Float2& center, float rInner, float startAngle, float angle, float thickness, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildCircleArc(bufferCreator(), lineCap, center, rInner, startAngle, angle, thickness, ColorFillDirection::InOut, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addCircleSegment(const Float2& center, float r, float startAngle, float angle, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildCircleSegment(bufferCreator(), center, r, startAngle, angle, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addEllipse(const Float2& center, float a, float b, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildEllipse(bufferCreator(), center, a, b, ColorFillDirection::InOut, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addEllipseFrame(const Float2& center, float a, float b, float innerThickness, float outerThickness, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildEllipseFrame(bufferCreator(), center, a, b, innerThickness, outerThickness, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addEllipsePie(const Float2& center, float rx, float ry, float startAngle, float angle, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildEllipsePie(bufferCreator(), center, rx, ry, startAngle, angle, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addSuperEllipse(const Float2& center, float a, float b, float n, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildSuperEllipse(bufferCreator(), center, a, b, n, ColorFillDirection::InOut, pattern.primaryColor, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addQuad(const FloatQuad& quad, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildQuad(bufferCreator(), quad, pattern.primaryColor), pattern);
		}
		void addRoundRect(const FloatRect& rect, float r, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildRoundRect(bufferCreator(), rect, r, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildRoundRectFrame(bufferCreator(), innerRect, innerR, outerRect, outerR, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addPolygon(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, const Optional<Float2>& offset, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildPolygon(bufferCreator(), vertices, triangleIndices, offset, pattern.primaryColor), pattern);
		}
		void addPolygon(std::span<const Float2> vertices, std::span<const Vertex2D::IndexType> indices, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildPolygon(bufferCreator(), vertices, indices, pattern.primaryColor), pattern);
		}
		void addPolygonTransformed(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, float s, float c, const Float2& offset, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildPolygonTransformed(bufferCreator(), vertices, triangleIndices, s, c, offset, pattern.primaryColor), pattern);
		}
		void addShape2DFrame(std::span<const Float2> vertices, const Optional<Float2>& offset, float thickness, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildShape2DFrame(bufferCreator(), vertices, offset, thickness, pattern.primaryColor, getRMSScaling()), pattern);
		}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildLineString(bufferCreator(), startCap, endCap, points, offset, thickness, inner, closeRing, pattern.primaryColor, getRMSScaling()), pattern);
		}
		// textured shapes: real (implemented in the .cpp via the texture program).
		void addTexturedCircle(const Texture& texture, const Circle& circle, const FloatRect& uv, const Float4& color) override;
		void addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4& color) override;
		void addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4(&colors)[4]) override;
		void addTexturedRoundRect(const Texture& texture, const FloatRect& rect, float w, float h, float r, const FloatRect& uvRect, const Float4& color) override;

		// shadows: tessellated by the common builder and sampled from the box-shadow
		// texture via the plain texture program (same as D3D11/Metal).
		void addCircleShadow(const Circle& circle, float blur, const Float4& color, bool fill) override
		{
			pushCommand(Vertex2DBuilder::BuildCircleShadow(bufferCreator(), circle, blur, color, getRMSScaling(), fill), Program::Texture, glTextureOf(*m_shadowTexture));
		}
		void addRectShadow(const FloatRect& rect, float blur, const Float4& color, bool fill) override
		{
			pushCommand(Vertex2DBuilder::BuildRectShadow(bufferCreator(), rect, blur, color, fill), Program::Texture, glTextureOf(*m_shadowTexture));
		}
		void addRoundRectShadow(const RoundRect& roundRect, float blur, const Float4& color, bool fill) override
		{
			pushCommand(Vertex2DBuilder::BuildRoundRectShadow(bufferCreator(), roundRect, blur, color, getRMSScaling(), fill), Program::Texture, glTextureOf(*m_shadowTexture));
		}

		// Mesh2D: caller-supplied vertices/indices, drawn with the shape or texture program.
		void addMesh2D(std::span<const Vertex2D> vertices, std::span<const TriangleIndex> indices, const Optional<Float2>& offset) override
		{
			discard(Vertex2DBuilder::BuildMesh2D(bufferCreator(), vertices, indices, offset));
		}
		void addMesh2D(const Texture& texture, std::span<const Vertex2D> vertices, std::span<const TriangleIndex> indices, const Optional<Float2>& offset) override
		{
			pushCommand(Vertex2DBuilder::BuildMesh2D(bufferCreator(), vertices, indices, offset), Program::Texture, glTextureOf(texture));
		}

		// Custom constant buffers are snapshotted per draw, like D3D11/Metal.
		void setConstantBuffer(ShaderStage stage, uint32 slot, const void* data, size_t size) override;
		uint32 beginConstantBufferScope(ShaderStage stage, uint32 slot, const void* data, size_t size) override;
		void endConstantBufferScope(ShaderStage stage, uint32 slot, uint32 previous) override;

		// render state: tracked here and snapshotted into each DrawCommand, then
		// applied per-command in flush(). VS sampler state is stored for fidelity
		// but never applied (this 2D renderer does no vertex-texture fetch).
		BlendState getBlendState() const override { return m_currentBlendState; }
		void setBlendState(const BlendState& state) override { m_currentBlendState = state; }
		RasterizerState getRasterizerState() const override { return m_currentRasterizerState; }
		void setRasterizerState(const RasterizerState& state) override { m_currentRasterizerState = state; }
		SamplerState getVSSamplerState(uint32 slot) const override { return m_vsSamplerStates[(slot < MaxSamplerSlots) ? slot : 0]; }
		void setVSSamplerState(uint32 slot, const SamplerState& state) override { if (slot < MaxSamplerSlots) { m_vsSamplerStates[slot] = state; } }
		SamplerState getPSSamplerState(uint32 slot) const override { return m_psSamplerStates[(slot < MaxSamplerSlots) ? slot : 0]; }
		void setPSSamplerState(uint32 slot, const SamplerState& state) override { if (slot < MaxSamplerSlots) { m_psSamplerStates[slot] = state; } }
		Optional<Rect> getScissorRect() const override { return m_currentScissorRect; }
		void setScissorRect(const Optional<Rect>& rect) override { m_currentScissorRect = rect; }
		Optional<Rect> getViewport() const override { return m_currentViewport; }
		void setViewport(const Optional<Rect>& viewport) override { m_currentViewport = viewport; }
		void setSDFParameters(const std::array<Float4, 3>& params) override { m_sdfParams = params; }
		Optional<VertexShader> getCustomVS() const override { return m_customVS; }
		void setCustomVS(const Optional<VertexShader>& vs) override { m_customVS = vs; }
		Optional<PixelShader> getCustomPS() const override { return m_customPS; }
		// A custom VS/PS (e.g. text's FontMSDF variants via ScopedCustomShader2D, or a
		// user shader) is captured per DrawCommand and bound in flush() instead of the
		// engine shader for the draw type.
		void setCustomPS(const Optional<PixelShader>& ps) override { m_customPS = ps; }
		const Texture& getShadowTexture() const noexcept override { return *m_shadowTexture; }

	private:

		[[nodiscard]]
		Vertex2DBufferPointer createBuffer(uint16 vertexCount, uint32 indexCount);

		// Callable adaptor for Vertex2DBuilder's BufferCreatorFunc (FunctionRef);
		// a temporary of this binds to the FunctionRef for the duration of a Build call.
		struct BufferCreator
		{
			CRenderer2D_GL4* self;

			Vertex2DBufferPointer operator()(const uint16 vertexCount, const uint32 indexCount) const
			{
				return self->createBuffer(vertexCount, indexCount);
			}
		};

		[[nodiscard]]
		BufferCreator bufferCreator() { return BufferCreator{ this }; }

		enum class Program : uint8
		{
			Shape,		// solid/gradient shapes
			Texture,	// sprites/emoji
			MSDF,		// text glyphs (custom PS active)
			Pattern,	// pattern fills (PatternType in patternType)
			Line,		// dashed/dotted line styles
		};

		// A run of indices drawn with one program + texture (+ pattern params) and a
		// snapshot of the render state in effect when it was recorded.
		struct DrawCommand
		{
			Program program = Program::Shape;
			GLuint texture = 0;
			uint32 indexCount = 0;
			std::array<Float4, 4> patternParams{};	// PatternParameters::toFloat4Array()
			uint8 patternType = 0;
			Mat3x3 transform = Mat3x3::Identity();	// (local * camera) * quad-warp, before the screen mapping
			BlendState blend{ BlendState::Default2D };
			RasterizerState rasterizer{ RasterizerState::Default2D };
			SamplerState sampler{};					// PS sampler, slot 0 (the only one applied)
			Optional<Rect> scissor;
			Optional<Rect> viewport;
			Optional<VertexShader::IDType> customVS;	// overrides the engine VS when set
			Optional<PixelShader::IDType> customPS;		// overrides the engine PS when set (e.g. text)
			Float4 colorMul{ 1.0f, 1.0f, 1.0f, 1.0f };
			Float3 colorAdd{ 0.0f, 0.0f, 0.0f };
			std::array<Float4, 3> sdfParams{};
			// Custom constant-buffer commands [0, constantBufferCommandEnd) precede this draw.
			uint32 constantBufferCommandEnd = 0;
		};

		// True when two commands share every piece of render state, so their index
		// runs can be drawn back-to-back without a state change.
		[[nodiscard]]
		bool sameState(const DrawCommand& a, const DrawCommand& b) const
		{
			return (a.program == b.program) && (a.texture == b.texture) && (a.patternType == b.patternType)
				&& (a.blend == b.blend) && (a.rasterizer.asValue() == b.rasterizer.asValue())
				&& (a.sampler.asValue() == b.sampler.asValue())
				&& (a.scissor == b.scissor) && (a.viewport == b.viewport) && (a.transform == b.transform)
				&& (a.customVS == b.customVS) && (a.customPS == b.customPS)
				&& (a.colorMul == b.colorMul) && (a.colorAdd == b.colorAdd) && (a.sdfParams == b.sdfParams);
		}

		// Stamp the current render state onto a command being recorded.
		void captureState(DrawCommand& command) const
		{
			command.blend		= m_currentBlendState;
			command.rasterizer	= m_currentRasterizerState;
			command.sampler		= m_psSamplerStates[0];
			command.scissor		= m_currentScissorRect;
			command.viewport	= m_currentViewport;
			command.transform	= (Mat3x3{ m_localTransform * m_cameraTransform } * m_quadWarpTransform);
			command.customVS	= (m_customVS ? Optional<VertexShader::IDType>{ m_customVS->id() } : none);
			command.customPS	= (m_customPS ? Optional<PixelShader::IDType>{ m_customPS->id() } : none);
			command.colorMul	= m_colorMul;
			command.colorAdd	= m_colorAdd;
			command.sdfParams	= m_sdfParams;
		}

		// Append `indexCount` indices, merging with the previous command when the
		// program, texture, subtype and render state all match.
		void pushCommand(const uint32 indexCount, Program program, GLuint texture, uint8 subType = 0)
		{
			if (indexCount == 0)
			{
				return;
			}

			DrawCommand command;
			command.program		= program;
			command.texture		= texture;
			command.indexCount	= indexCount;
			command.patternType	= subType;
			captureState(command);
			const bool constantBuffersChanged = recordConstantBuffers(command);

			if ((not constantBuffersChanged) && (not m_commands.isEmpty()) && sameState(m_commands.back(), command))
			{
				m_commands.back().indexCount += indexCount;
			}
			else
			{
				m_commands.push_back(command);
			}
		}

		void discard(const uint32 indexCount) { pushCommand(indexCount, Program::Shape, 0); } // shape draw

		// Pattern fills carry per-draw params, so each is its own command (no merge).
		void pushPatternCommand(const uint32 indexCount, const PatternParameters& pattern)
		{
			if (indexCount == 0)
			{
				return;
			}

			DrawCommand command;
			command.program			= Program::Pattern;
			command.indexCount		= indexCount;
			command.patternParams	= pattern.toFloat4Array();
			command.patternType		= static_cast<uint8>(FromEnum(pattern.type));
			captureState(command);
			recordConstantBuffers(command);
			m_commands.push_back(command);
		}

		// Commits pending custom constant-buffer changes ahead of `command`; returns
		// whether there were any (the command then cannot merge with the previous one).
		bool recordConstantBuffers(DrawCommand& command)
		{
			const bool changed = m_constantBuffers.hasStateChange();

			if (changed)
			{
				m_constantBuffers.flush();
			}

			command.constantBufferCommandEnd = static_cast<uint32>(m_constantBuffers.size());
			return changed;
		}

		// Uploads one recorded custom constant buffer and binds it to its stage slot.
		void applyConstantBuffer(const ConstantBuffer2DCommand& command);

		// Cached like the D3D11 backend: getRMSScaling() runs on every tessellated draw.
		void updateRMSScaling() noexcept { m_rmsScaling = (m_localTransform * m_cameraTransform).rmsScaling(); }

		// GL texture name for a Texture handle (via the Linux CTexture backend).
		[[nodiscard]]
		GLuint glTextureOf(const Texture& texture);

		// GL sampler object for a SamplerState, created and cached on first use.
		[[nodiscard]]
		GLuint samplerObjectFor(const SamplerState& state);

		static constexpr uint32 MaxSamplerSlots = 8;

		Array<Vertex2D> m_vertices;

		Array<Vertex2D::IndexType> m_indices;

		Array<DrawCommand> m_commands;

		// Box-shadow sprite sampled by addCircleShadow/addRectShadow/addRoundRectShadow.
		std::unique_ptr<Texture> m_shadowTexture;

		Float4 m_colorMul = { 1.0f, 1.0f, 1.0f, 1.0f };

		Float3 m_colorAdd = { 0.0f, 0.0f, 0.0f };

		Mat3x2 m_localTransform = Mat3x2::Identity();

		Mat3x2 m_cameraTransform = Mat3x2::Identity();

		Mat3x3 m_quadWarpTransform = Mat3x3::Identity();

		float m_rmsScaling = 1.0f;

		// Current render state (snapshotted into each DrawCommand by captureState).
		BlendState m_currentBlendState{ BlendState::Default2D };

		RasterizerState m_currentRasterizerState{ RasterizerState::Default2D };

		std::array<SamplerState, MaxSamplerSlots> m_psSamplerStates{};

		std::array<SamplerState, MaxSamplerSlots> m_vsSamplerStates{};

		Optional<Rect> m_currentScissorRect;

		Optional<Rect> m_currentViewport;

		// SamplerState (asValue) -> GL sampler object.
		std::unordered_map<uint64, GLuint> m_samplerCache;

		// 0 when the context has no anisotropic filtering extension.
		GLfloat m_maxAnisotropy = 0.0f;

		GLuint m_vao = 0;

		GLuint m_vbo = 0;

		GLuint m_ibo = 0;

		// Engine constant buffers (UBOs), bound to their block binding points
		// (VSConstants2D@0, PSConstants2D@14, PSEffectConstants2D@15) and updated
		// per DrawCommand in flush().
		ConstantBuffer<VSConstants2D> m_vsConstants;

		ConstantBuffer<PSConstants2D> m_psConstants;

		ConstantBuffer<PSEffectConstants2D> m_psEffectConstants;

		// SDF text parameters (set via setSDFParameters), fed into PSConstants2D.
		std::array<Float4, 3> m_sdfParams{ Float4{ 0.5f, 0.5f, 0.0f, 0.0f }, Float4{ 0.0f, 0.0f, 0.0f, 1.0f }, Float4{ 0.0f, 0.0f, 0.0f, 0.5f } };

		// Active custom shaders (text's FontMSDF variants, or a user shader),
		// captured into each DrawCommand and bound in flush().
		Optional<VertexShader> m_customVS;

		Optional<PixelShader> m_customPS;

		ConstantBuffer2DCommands m_constantBuffers;

		struct CustomConstantBuffer
		{
			std::unique_ptr<IConstantBuffer> buffer;

			size_t capacity = 0;
		};

		// [stage][slot] upload buffers for custom constant buffers, grown to powers of two.
		std::array<std::array<CustomConstantBuffer, Graphics::ConstantBufferSlotCount>, 2> m_customConstantBuffers;
	};
}
