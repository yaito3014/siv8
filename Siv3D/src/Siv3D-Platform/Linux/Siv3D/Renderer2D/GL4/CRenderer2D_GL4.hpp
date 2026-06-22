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
# include <Siv3D/Renderer2D/IRenderer2D.hpp>
# include <Siv3D/Renderer2D/Vertex2DBufferPointer.hpp>
# include <Siv3D/Array.hpp>
# include <Siv3D/Mat3x2.hpp>
# include <Siv3D/Texture.hpp>
# include <Siv3D/BlendState.hpp>
# include <Siv3D/RasterizerState.hpp>
# include <Siv3D/SamplerState.hpp>
# include <Siv3D/Common/OpenGL.hpp>

namespace s3d
{
	// Phase 1 (in progress): a minimal real 2D renderer. Solid-colored shapes are
	// tessellated by the common Vertex2DBuilder and drawn with a single shape
	// program; patterns/textures/text are still no-op (TODO(linux)).
	class CRenderer2D_GL4 final : public ISiv3DRenderer2D
	{
	public:

		~CRenderer2D_GL4() override;

		void init() override;

		// --- implemented (solid shapes) ---
		void addTriangle(const Float2(&points)[3], const Float4& color) override;
		void addTriangle(const Float2(&points)[3], const Float4(&colors)[3]) override;
		void addRect(const FloatRect& rect, const Float4& color) override;
		void addRect(const FloatRect& rect, const Float4(&colors)[4]) override;
		void addCircle(const Float2& center, float r, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override;
		void addLine(LineCap startCap, LineCap endCap, const Float2& start, const Float2& end, float thickness, const Float4(&colors)[2]) override;

		void flush() override;

		Float4 getColorMul() const override;
		void setColorMul(const Float4& color) override;
		Float3 getColorAdd() const override;
		void setColorAdd(const Float3& color) override;
		float getMaxScaling() const noexcept override;
		const Mat3x2& getLocalTransform() const override;
		void setLocalTransform(const Mat3x2& matrix) override;
		const Mat3x2& getCameraTransform() const override;
		void setCameraTransform(const Mat3x2& matrix) override;

		// --- not yet implemented (Phase 1+): no-op so the engine links/runs ---
		void addLine(const LineStyle& style, const Float2& start, const Float2& end, float thickness, const Float4(&colors)[2]) override {}
		void addArrow(LineCap startCap, const Float2& start, const Float2& end, float thickness, const Float2& headSize, const Float4(&colors)[2]) override {}
		void addTriangle(const Float2(&points)[3], const PatternParameters& pattern) override {}
		void addRect(const FloatRect& rect, const PatternParameters& pattern) override {}
		void addRectFrame(const FloatRect& innerRect, float thickness, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override {}
		void addRectFrame(const FloatRect& innerRect, float thickness, const PatternParameters& pattern) override {}
		void addCircle(const Float2& center, float r, const PatternParameters& pattern) override {}
		void addCircleFrame(const Float2& center, float rInner, float thickness, const Float4& innerColor, const Float4& outerColor) override {}
		void addCircleFrame(const Float2& center, float rInner, float thickness, const PatternParameters& pattern) override {}
		void addCirclePie(const Float2& center, float r, float startAngle, float angle, const Float4& innerColor, const Float4& outerColor) override {}
		void addCirclePie(const Float2& center, float r, float startAngle, float angle, const PatternParameters& pattern) override {}
		void addCircleArc(LineCap lineCap, const Float2& center, float rInner, float startAngle, float angle, float thickness, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override {}
		void addCircleArc(LineCap lineCap, const Float2& center, float rInner, float startAngle, float angle, float thickness, const PatternParameters& pattern) override {}
		void addCircleSegment(const Float2& center, float r, float startAngle, float angle, const Float4& color) override {}
		void addCircleSegment(const Float2& center, float r, float startAngle, float angle, const PatternParameters& pattern) override {}
		void addEllipse(const Float2& center, float a, float b, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override {}
		void addEllipse(const Float2& center, float a, float b, const PatternParameters& pattern) override {}
		void addEllipseFrame(const Float2& center, float a, float b, float innerThickness, float outerThickness, const Float4& innerColor, const Float4& outerColor) override {}
		void addEllipseFrame(const Float2& center, float a, float b, float innerThickness, float outerThickness, const PatternParameters& pattern) override {}
		void addEllipsePie(const Float2& center, float rx, float ry, float startAngle, float angle, const Float4& innerColor, const Float4& outerColor) override {}
		void addEllipsePie(const Float2& center, float rx, float ry, float startAngle, float angle, const PatternParameters& pattern) override {}
		void addSuperEllipse(const Float2& center, float a, float b, float n, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override {}
		void addSuperEllipse(const Float2& center, float a, float b, float n, const PatternParameters& pattern) override {}
		void addQuad(const FloatQuad& quad, const Float4& color) override {}
		void addQuad(const FloatQuad& quad, const Float4(&colors)[4]) override {}
		void addQuad(const FloatQuad& quad, const PatternParameters& pattern) override {}
		void addRoundRect(const FloatRect& rect, float r, const Float4& color) override {}
		void addRoundRect(const FloatRect& rect, float r, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override {}
		void addRoundRect(const FloatRect& rect, float r, const PatternParameters& pattern) override {}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const Float4& color) override {}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const Float4& color0, const Float4& color1, ColorFillDirection colorType) override {}
		void addRoundRectFrame(const FloatRect& innerRect, const float innerR, const FloatRect& outerRect, const float outerR, const PatternParameters& pattern) override {}
		void addPolygon(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, const Optional<Float2>& offset, const Float4& color) override {}
		void addPolygon(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, const Optional<Float2>& offset, const PatternParameters& pattern) override {}
		void addPolygon(std::span<const Float2> vertices, std::span<const Vertex2D::IndexType> indices, const Float4& color) override {}
		void addPolygon(std::span<const Float2> vertices, std::span<const Vertex2D::IndexType> indices, const PatternParameters& pattern) override {}
		void addPolygonTransformed(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, float s, float c, const Float2& offset, const Float4& color) override {}
		void addPolygonTransformed(std::span<const Float2> vertices, std::span<const TriangleIndex> triangleIndices, float s, float c, const Float2& offset, const PatternParameters& pattern) override {}
		void addShape2DFrame(std::span<const Float2> vertices, float thickness, const Float4& color) override {}
		void addShape2DFrame(std::span<const Float2> vertices, float thickness, const PatternParameters& pattern) override {}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, const Float4& color) override {}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, const Float4& colorStart, const Float4& colorEnd) override {}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, const PatternParameters& pattern) override {}
		void addLineString(LineCap startCap, LineCap endCap, std::span<const Vec2> points, const Optional<Float2>& offset, float thickness, bool inner, CloseRing closeRing, std::span<const ColorF> colors) override {}
		void addTexturedCircle(const Texture& texture, const Circle& circle, const FloatRect& uv, const Float4& color) override {}
		void addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4& color) override {}
		void addTexturedQuad(const Texture& texture, const FloatQuad& quad, const FloatRect& uv, const Float4(&colors)[4]) override {}
		void addTexturedRoundRect(const Texture& texture, const FloatRect& rect, float w, float h, float r, const FloatRect& uvRect, const Float4& color) override {}
		void addCircleShadow(const Circle& circle, float blur, const Float4& color, bool fill) override {}
		void addRectShadow(const FloatRect& rect, float blur, const Float4& color, bool fill) override {}
		void addRoundRectShadow(const RoundRect& roundRect, float blur, const Float4& color, bool fill) override {}
		void addQuadWarp(const Texture& texture, const FloatRect& uv, const FloatQuad& quad, const Float4& color) override {}
		void addQuadWarp(const Texture& texture, const FloatRect& uv, const FloatQuad& quad, const Float4(&colors)[4]) override {}
		BlendState getBlendState() const override { return {}; }
		void setBlendState(const BlendState& state) override {}
		RasterizerState getRasterizerState() const override { return {}; }
		void setRasterizerState(const RasterizerState& state) override {}
		SamplerState getVSSamplerState(uint32 slot) const override { return {}; }
		void setVSSamplerState(uint32 slot, const SamplerState& state) override {}
		SamplerState getPSSamplerState(uint32 slot) const override { return {}; }
		void setPSSamplerState(uint32 slot, const SamplerState& state) override {}
		Optional<Rect> getScissorRect() const override { return {}; }
		void setScissorRect(const Optional<Rect>& rect) override {}
		Optional<Rect> getViewport() const override { return {}; }
		void setViewport(const Optional<Rect>& viewport) override {}
		void setSDFParameters(const std::array<Float4, 3>& params) override {}
		Optional<VertexShader> getCustomVS() const override { return {}; }
		void setCustomVS(const Optional<VertexShader>& vs) override {}
		Optional<PixelShader> getCustomPS() const override { return {}; }
		void setCustomPS(const Optional<PixelShader>& ps) override {}
		const Texture& getShadowTexture() const noexcept override { static const Texture t; return t; }

	private:

		[[nodiscard]]
		Vertex2DBufferPointer createBuffer(Vertex2D::IndexType vertexCount, Vertex2D::IndexType indexCount);

		Array<Vertex2D> m_vertices;

		Array<Vertex2D::IndexType> m_indices;

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
	};
}
