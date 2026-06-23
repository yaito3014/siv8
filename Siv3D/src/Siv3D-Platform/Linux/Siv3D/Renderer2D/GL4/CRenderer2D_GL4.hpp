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
# include <Siv3D/Renderer2D/IRenderer2D.hpp>
# include <Siv3D/Renderer2D/Vertex2DBuilder.hpp>
# include <Siv3D/Renderer2D/Vertex2DBufferPointer.hpp>
# include <Siv3D/Pattern/PatternParameters.hpp>
# include <Siv3D/LineStyle.hpp>
# include <Siv3D/Array.hpp>
# include <Siv3D/Mat3x2.hpp>
# include <Siv3D/Texture.hpp>
# include <Siv3D/BlendState.hpp>
# include <Siv3D/RasterizerState.hpp>
# include <Siv3D/SamplerState.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace s3d
{
	// Phase 1: a working 2D renderer. Solid/gradient shapes are tessellated by the
	// common Vertex2DBuilder; textures/sprites, MSDF text, the six fill patterns,
	// and dashed/dotted lines all render. Still no-op (TODO(linux)): pattern-fill
	// frames/arcs/complex shapes, shadows, quad-warp, render state (blend/
	// rasterizer/sampler/scissor/viewport), SDF outline/glow, and arbitrary custom
	// shaders. See the "partially implemented" section below for the exact split.
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
			discard(Vertex2DBuilder::BuildLine(bufferCreator(), startCap, endCap, start, end, thickness, colors, getMaxScaling()));
		}
		void addLine(const LineStyle& style, const Float2& start, const Float2& end, float thickness, const Float4(&colors)[2]) override
		{
			const Vertex2D::IndexType indexCount = Vertex2DBuilder::BuildLine(bufferCreator(), style, start, end, thickness, colors, getMaxScaling());

			if (style.type == LineType::Solid)
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
			discard(Vertex2DBuilder::BuildArrow(bufferCreator(), startCap, start, end, thickness, headSize, colors, getMaxScaling()));
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
		void addCircle(const Float2& center, float r, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildCircle(bufferCreator(), center, r, colorType, color0, color1, getMaxScaling()));
		}
		void addCircleFrame(const Float2& center, float rInner, float thickness, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildCircleFrame(bufferCreator(), center, rInner, thickness, innerColor, outerColor, getMaxScaling()));
		}
		void addCirclePie(const Float2& center, float r, float startAngle, float angle, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildCirclePie(bufferCreator(), center, r, startAngle, angle, innerColor, outerColor, getMaxScaling()));
		}
		void addCircleArc(LineCap lineCap, const Float2& center, float rInner, float startAngle, float angle, float thickness, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildCircleArc(bufferCreator(), lineCap, center, rInner, startAngle, angle, thickness, colorType, color0, color1, getMaxScaling()));
		}
		void addCircleSegment(const Float2& center, float r, float startAngle, float angle, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildCircleSegment(bufferCreator(), center, r, startAngle, angle, color, getMaxScaling()));
		}
		void addEllipse(const Float2& center, float a, float b, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildEllipse(bufferCreator(), center, a, b, colorType, color0, color1, getMaxScaling()));
		}
		void addEllipseFrame(const Float2& center, float a, float b, float innerThickness, float outerThickness, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildEllipseFrame(bufferCreator(), center, a, b, innerThickness, outerThickness, innerColor, outerColor, getMaxScaling()));
		}
		void addEllipsePie(const Float2& center, float rx, float ry, float startAngle, float angle, const Float4& innerColor, const Float4& outerColor) override
		{
			discard(Vertex2DBuilder::BuildEllipsePie(bufferCreator(), center, rx, ry, startAngle, angle, innerColor, outerColor, getMaxScaling()));
		}
		void addSuperEllipse(const Float2& center, float a, float b, float n, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildSuperEllipse(bufferCreator(), center, a, b, n, colorType, color0, color1, getMaxScaling()));
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
			discard(Vertex2DBuilder::BuildRoundRect(bufferCreator(), rect, r, color, getMaxScaling()));
		}
		void addRoundRect(const FloatRect& rect, float r, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildRoundRect(bufferCreator(), rect, r, colorType, color0, color1, getMaxScaling()));
		}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildRoundRectFrame(bufferCreator(), innerRect, innerR, outerRect, outerR, color, getMaxScaling()));
		}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override
		{
			discard(Vertex2DBuilder::BuildRoundRectFrame(bufferCreator(), innerRect, innerR, outerRect, outerR, colorType, color0, color1, getMaxScaling()));
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
		void addShape2DFrame(std::span<const Float2> vertices, float thickness, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildShape2DFrame(bufferCreator(), vertices, thickness, color, getMaxScaling()));
		}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, const Float4& color) override
		{
			discard(Vertex2DBuilder::BuildLineString(bufferCreator(), startCap, endCap, points, offset, thickness, inner, closeRing, color, getMaxScaling()));
		}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, const Float4& colorStart, const Float4& colorEnd) override
		{
			discard(Vertex2DBuilder::BuildLineString(bufferCreator(), startCap, endCap, points, offset, thickness, inner, colorStart, colorEnd, getMaxScaling()));
		}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, std::span<const ColorF> colors) override
		{
			discard(Vertex2DBuilder::BuildLineString(bufferCreator(), startCap, endCap, points, offset, thickness, inner, closeRing, colors, getMaxScaling()));
		}

		////////////////////////////////////////////////////////////////
		//	render state
		////////////////////////////////////////////////////////////////

		Float4 getColorMul() const override { return m_colorMul; }
		void setColorMul(const Float4& color) override { m_colorMul = color; }
		Float3 getColorAdd() const override { return m_colorAdd; }
		void setColorAdd(const Float3& color) override { m_colorAdd = color; }
		float getMaxScaling() const noexcept override { return 1.0f; }
		const Mat3x2& getLocalTransform() const override { return m_localTransform; }
		void setLocalTransform(const Mat3x2& matrix) override { m_localTransform = matrix; }
		const Mat3x2& getCameraTransform() const override { return m_cameraTransform; }
		void setCameraTransform(const Mat3x2& matrix) override { m_cameraTransform = matrix; }

		////////////////////////////////////////////////////////////////
		//	partially implemented: pattern fills, textured shapes, shadows,
		//	quad-warp, render state, and custom shaders. Methods carrying real
		//	work are noted; the bare `{}` bodies are no-op TODO(linux) (they
		//	keep the engine linking/running and silently drop the draw).
		////////////////////////////////////////////////////////////////

		// pattern fills: triangle/rect/circle work; frames/arcs/pies and the
		// other complex shapes are not yet routed through the pattern program.
		void addTriangle(const Float2(&points)[3], const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildTriangle(bufferCreator(), points, pattern.primaryColor), pattern);
		}
		void addRect(const FloatRect& rect, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildRect(bufferCreator(), rect, pattern.primaryColor), pattern);
		}
		void addRectFrame(const FloatRect& innerRect, float thickness, const PatternParameters& pattern) override {}
		void addCircle(const Float2& center, float r, const PatternParameters& pattern) override
		{
			pushPatternCommand(Vertex2DBuilder::BuildCircle(bufferCreator(), center, r, ColorFillDirection::InOut, pattern.primaryColor, pattern.primaryColor, getMaxScaling()), pattern);
		}
		void addCircleFrame(const Float2& center, float rInner, float thickness, const PatternParameters& pattern) override {}
		void addCirclePie(const Float2& center, float r, float startAngle, float angle, const PatternParameters& pattern) override {}
		void addCircleArc(LineCap lineCap, const Float2& center, float rInner, float startAngle, float angle, float thickness, const PatternParameters& pattern) override {}
		void addCircleSegment(const Float2& center, float r, float startAngle, float angle, const PatternParameters& pattern) override {}
		void addEllipse(const Float2& center, float a, float b, const PatternParameters& pattern) override {}
		void addEllipseFrame(const Float2& center, float a, float b, float innerThickness, float outerThickness, const PatternParameters& pattern) override {}
		void addEllipsePie(const Float2& center, float rx, float ry, float startAngle, float angle, const PatternParameters& pattern) override {}
		void addSuperEllipse(const Float2& center, float a, float b, float n, const PatternParameters& pattern) override {}
		void addQuad(const FloatQuad& quad, const PatternParameters& pattern) override {}
		void addRoundRect(const FloatRect& rect, float r, const PatternParameters& pattern) override {}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const PatternParameters& pattern) override {}
		void addPolygon(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, const Optional<Float2>& offset, const PatternParameters& pattern) override {}
		void addPolygon(std::span<const Float2> vertices, std::span<const Vertex2D::IndexType> indices, const PatternParameters& pattern) override {}
		void addPolygonTransformed(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, float s, float c, const Float2& offset, const PatternParameters& pattern) override {}
		void addShape2DFrame(std::span<const Float2> vertices, float thickness, const PatternParameters& pattern) override {}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, const PatternParameters& pattern) override {}
		// textured shapes: real (implemented in the .cpp via the texture program).
		void addTexturedCircle(const Texture& texture, const Circle& circle, const FloatRect& uv, const Float4& color) override;
		void addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4& color) override;
		void addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4(&colors)[4]) override;
		void addTexturedRoundRect(const Texture& texture, const FloatRect& rect, float w, float h, float r, const FloatRect& uvRect, const Float4& color) override;

		// shadows + quad-warp: no-op (TODO(linux)).
		void addCircleShadow(const Circle& circle, float blur, const Float4& color, bool fill) override {}
		void addRectShadow(const FloatRect& rect, float blur, const Float4& color, bool fill) override {}
		void addRoundRectShadow(const RoundRect& roundRect, float blur, const Float4& color, bool fill) override {}
		void addQuadWarp(const Texture& texture, const FloatRect& uv, const FloatQuad& quad, const Float4& color) override {}
		void addQuadWarp(const Texture& texture, const FloatRect& uv, const FloatQuad& quad, const Float4(&colors)[4]) override {}

		// render state: not honored yet (GL pipeline state is fixed). Getters
		// return defaults, setters are ignored (TODO(linux)).
		BlendState getBlendState() const override { return{}; }
		void setBlendState(const BlendState& state) override {}
		RasterizerState getRasterizerState() const override { return{}; }
		void setRasterizerState(const RasterizerState& state) override {}
		SamplerState getVSSamplerState(uint32 slot) const override { return{}; }
		void setVSSamplerState(uint32 slot, const SamplerState& state) override {}
		SamplerState getPSSamplerState(uint32 slot) const override { return{}; }
		void setPSSamplerState(uint32 slot, const SamplerState& state) override {}
		Optional<Rect> getScissorRect() const override { return{}; }
		void setScissorRect(const Optional<Rect>& rect) override {}
		Optional<Rect> getViewport() const override { return{}; }
		void setViewport(const Optional<Rect>& viewport) override {}
		void setSDFParameters(const std::array<Float4, 3>& params) override {}
		Optional<VertexShader> getCustomVS() const override { return{}; }
		void setCustomVS(const Optional<VertexShader>& vs) override {}
		Optional<PixelShader> getCustomPS() const override { return{}; }
		// The only custom-PS user in practice is text (ScopedCustomShader2D wraps
		// the FontMSDF shader), so an active custom PS routes textured draws to
		// the MSDF program. TODO(linux): honor arbitrary user pixel shaders.
		void setCustomPS(const Optional<PixelShader>& ps) override { m_customPSActive = ps.has_value(); }
		const Texture& getShadowTexture() const noexcept override { static const Texture t; return t; }

	private:

		[[nodiscard]]
		Vertex2DBufferPointer createBuffer(Vertex2D::IndexType vertexCount, Vertex2D::IndexType indexCount);

		// Callable adaptor for Vertex2DBuilder's BufferCreatorFunc (FunctionRef);
		// a temporary of this binds to the FunctionRef for the duration of a Build call.
		struct BufferCreator
		{
			CRenderer2D_GL4* self;

			Vertex2DBufferPointer operator()(Vertex2D::IndexType vertexCount, Vertex2D::IndexType indexCount) const
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
			Pattern,	// checker/grid/polka-dot fills
			Line,		// dashed/dotted line styles
		};

		// A run of indices drawn with one program + texture (+ pattern params).
		struct DrawCommand
		{
			Program program = Program::Shape;
			GLuint texture = 0;
			uint32 indexCount = 0;
			std::array<Float4, 3> patternParams{};	// (uvTransform packed, params, backgroundColor)
			uint8 patternType = 0;
		};

		// Append `indexCount` indices, merging with the previous command when the
		// program, texture and subtype (line/pattern type) all match.
		void pushCommand(Vertex2D::IndexType indexCount, Program program, GLuint texture, uint8 subType = 0)
		{
			if (indexCount == 0)
			{
				return;
			}

			if ((not m_commands.isEmpty()) && (m_commands.back().program == program)
				&& (m_commands.back().texture == texture) && (m_commands.back().patternType == subType))
			{
				m_commands.back().indexCount += indexCount;
			}
			else
			{
				DrawCommand command;
				command.program		= program;
				command.texture		= texture;
				command.indexCount	= indexCount;
				command.patternType	= subType;
				m_commands.push_back(command);
			}
		}

		void discard(Vertex2D::IndexType indexCount) { pushCommand(indexCount, Program::Shape, 0); } // shape draw

		// Pattern fills carry per-draw params, so each is its own command (no merge).
		void pushPatternCommand(Vertex2D::IndexType indexCount, const PatternParameters& pattern)
		{
			if (indexCount == 0)
			{
				return;
			}

			DrawCommand command;
			command.program			= Program::Pattern;
			command.indexCount		= indexCount;
			command.patternParams	= pattern.toFloat4Array(1.0f / getMaxScaling());
			command.patternType		= static_cast<uint8>(FromEnum(pattern.type));
			m_commands.push_back(command);
		}

		// GL texture name for a Texture handle (via the Linux CTexture backend).
		[[nodiscard]]
		GLuint glTextureOf(const Texture& texture);

		Array<Vertex2D> m_vertices;

		Array<Vertex2D::IndexType> m_indices;

		Array<DrawCommand> m_commands;

		Float4 m_colorMul = { 1.0f, 1.0f, 1.0f, 1.0f };

		Float3 m_colorAdd = { 0.0f, 0.0f, 0.0f };

		Mat3x2 m_localTransform = Mat3x2::Identity();

		Mat3x2 m_cameraTransform = Mat3x2::Identity();

		GLuint m_vao = 0;

		GLuint m_vbo = 0;

		GLuint m_ibo = 0;

		GLuint m_program = 0;

		GLint m_locTransform0 = -1;

		GLint m_locTransform1 = -1;

		GLint m_locColorMul = -1;

		GLuint m_textureProgram = 0;

		GLint m_texLocTransform0 = -1;

		GLint m_texLocTransform1 = -1;

		GLint m_texLocColorMul = -1;

		GLint m_texLocSampler = -1;

		GLuint m_msdfProgram = 0;

		GLint m_msdfLocTransform0 = -1;

		GLint m_msdfLocTransform1 = -1;

		GLint m_msdfLocColorMul = -1;

		GLint m_msdfLocSampler = -1;

		GLuint m_patternProgram = 0;

		GLint m_patLocTransform0 = -1;

		GLint m_patLocTransform1 = -1;

		GLint m_patLocColorMul = -1;

		GLint m_patLocPt0 = -1;

		GLint m_patLocPt1 = -1;

		GLint m_patLocBg = -1;

		GLint m_patLocType = -1;

		GLint m_patLocFbHeight = -1;

		GLuint m_lineProgram = 0;

		GLint m_lineLocTransform0 = -1;

		GLint m_lineLocTransform1 = -1;

		GLint m_lineLocColorMul = -1;

		GLint m_lineLocType = -1;

		bool m_customPSActive = false;
	};
}
