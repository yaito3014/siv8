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
# include <string>
# include <string_view>

//	GLSL 4.1 ports of the engine's built-in 2D shaders (faithful to
//	App/app/engine/shader/d3d11/2d.hlsl). Each shader is assembled from a shared
//	prefix + a per-shader main(). std140 uniform blocks mirror the engine
//	constant structs; binding points are assigned via glUniformBlockBinding in
//	CShader_GL4 (VSConstants2D@0, PSConstants2D@8, PSEffectConstants2D@9).
//
//	Conventions matching the GL4 renderer:
//	- vertex attribute locations 0=position, 1=uv, 2=color (the VAO layout).
//	- varyings carry explicit locations so separable VS/PS pair in the pipeline.
//	- patterns read gl_FragCoord with layout(origin_upper_left) so the math
//	  matches D3D11 SV_Position (top-left pixel origin); no manual Y-flip.
//	- textures are sampled with v_uv directly (CTexture_GL4 uploads un-flipped).

namespace s3d::detail
{
	//
	//	Vertex shaders
	//
	constexpr std::string_view GLSL_VSPrefix =
R"glsl(#version 410 core
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) in vec2 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec4 i_color;
layout(location = 0) out vec4 v_colorPMA;
layout(location = 1) out vec2 v_uv;
layout(std140) uniform VSConstants2D
{
	vec4 g_transform[2];
	vec4 g_colorMul;
};
vec4 s3d_positionTransform(vec2 pos)
{
	return vec4((vec2(g_transform[0].z, g_transform[0].w)
		+ (pos.x * vec2(g_transform[0].x, g_transform[0].y))
		+ (pos.y * vec2(g_transform[1].x, g_transform[1].y))), g_transform[1].z, g_transform[1].w);
}
vec4 s3d_premultiplyAlpha(vec4 c) { return vec4((c.rgb * c.a), c.a); }
)glsl";

	constexpr std::string_view GLSL_VS_Shape =
R"glsl(void main()
{
	gl_Position = s3d_positionTransform(i_position);
	v_colorPMA = s3d_premultiplyAlpha(i_color * g_colorMul);
	v_uv = i_uv;
}
)glsl";

	constexpr std::string_view GLSL_VS_QuadWarp =
R"glsl(void main()
{
	gl_Position = s3d_positionTransform(i_position);
	v_colorPMA = s3d_premultiplyAlpha(i_color * g_colorMul);
	v_uv = i_position;
}
)glsl";

	// Fullscreen triangle: no vertex inputs, generated from gl_VertexID.
	constexpr std::string_view GLSL_VS_FullScreenTriangle =
R"glsl(#version 410 core
out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec2 v_uv;
void main()
{
	gl_Position = vec4((gl_VertexID == 2 ? 3.0 : -1.0), (gl_VertexID == 1 ? 3.0 : -1.0), 0.0, 1.0);
	v_uv = vec2((gl_VertexID == 2 ? 2.0 : 0.0), (gl_VertexID == 1 ? -1.0 : 1.0));
}
)glsl";

	//
	//	Pixel shaders
	//
	constexpr std::string_view GLSL_PSPrefix =
R"glsl(#version 410 core
layout(location = 0) in vec4 v_colorPMA;
layout(location = 1) in vec2 v_uv;
out vec4 o_color;
uniform sampler2D g_texture0;
layout(std140) uniform PSConstants2D
{
	vec4 g_patternBackgroundColorMul;
	vec4 g_colorAdd;
	vec4 g_sdfParam;
	vec4 g_sdfOutlineColorPMA;
	vec4 g_sdfShadowColorPMA;
};
vec4 s3d_premultiplyAlpha(vec4 c) { return vec4((c.rgb * c.a), c.a); }
vec4 s3d_shapeColor(vec4 c) { return (c + (g_colorAdd * c.a)); }
vec4 s3d_textureColor(vec4 v, vec4 t) { v *= t; return (v + (g_colorAdd * v.a)); }
)glsl";

	// Pattern prefix: adds the effect block + pattern helpers, and reads
	// gl_FragCoord with a top-left origin to match the D3D11 math.
	constexpr std::string_view GLSL_PSPatternPrefix =
R"glsl(layout(origin_upper_left) in vec4 gl_FragCoord;
layout(std140) uniform PSEffectConstants2D
{
	vec4 g_patternUVTransform[2];
	vec4 g_patternBackgroundColor;
	vec4 g_quadWarpInvHomography[3];
	vec4 g_quadWarpUVTransform;
};
vec4 Pattern_BackgroundColor()
{
	vec4 colorPMA = s3d_premultiplyAlpha(g_patternBackgroundColor * g_patternBackgroundColorMul);
	return (colorPMA + (g_colorAdd * colorPMA.a));
}
vec2 Pattern_UVTransform(vec2 uv)
{
	return (vec2(g_patternUVTransform[0].z, g_patternUVTransform[0].w)
		+ (uv.x * vec2(g_patternUVTransform[0].x, g_patternUVTransform[0].y))
		+ (uv.y * vec2(g_patternUVTransform[1].x, g_patternUVTransform[1].y)));
}
vec2 Pattern_Integral(vec2 v)
{
	v /= 2.0;
	return (floor(v) + max((2.0 * fract(v) - 1.0), 0.0));
}
float Pattern_CheckersFiltered(vec2 p, vec2 hv)
{
	vec2 fw = fwidth(p);
	float w = max(fw.x, fw.y);
	vec2 i = (Pattern_Integral(p + 0.5 * w) - Pattern_Integral(p - 0.5 * w));
	i *= hv;
	i /= w;
	return (i.x + i.y - 2.0 * i.x * i.y);
}
vec2 Pattern_Skew(vec2 v)
{
	return vec2((v.x + (v.y * 0.57735027)), (v.y * 1.15470054));
}
float Pattern_Hex(vec2 p)
{
	vec2 HEX = vec2(1.0, 1.73205081);
	vec4 t = (floor(vec4(p, p - vec2(0.5, 1.0)) / HEX.xyxy) + vec4(0.5));
	vec4 h = vec4((p - t.xy * HEX), (p - (t.zw + vec2(0.5)) * HEX));
	vec2 hx = abs((dot(h.xy, h.xy) < dot(h.zw, h.zw)) ? h.xy : h.zw);
	return max(dot(hx, (HEX * 0.5)), hx.x);
}
)glsl";

	constexpr std::string_view GLSL_PS_Shape =
R"glsl(void main()
{
	o_color = s3d_shapeColor(v_colorPMA);
}
)glsl";

	constexpr std::string_view GLSL_PS_Texture =
R"glsl(void main()
{
	o_color = s3d_textureColor(v_colorPMA, texture(g_texture0, v_uv));
}
)glsl";

	constexpr std::string_view GLSL_PS_QuadWarp =
R"glsl(layout(std140) uniform PSEffectConstants2D
{
	vec4 g_patternUVTransform[2];
	vec4 g_patternBackgroundColor;
	vec4 g_quadWarpInvHomography[3];
	vec4 g_quadWarpUVTransform;
};
void main()
{
	vec3 v = vec3(v_uv, 1.0);
	vec3 t = (v.x * g_quadWarpInvHomography[0].xyz)
		+ (v.y * g_quadWarpInvHomography[1].xyz)
		+ (v.z * g_quadWarpInvHomography[2].xyz);
	vec2 uv = ((t.xy / t.z) * g_quadWarpUVTransform.xy + g_quadWarpUVTransform.zw);
	o_color = s3d_textureColor(v_colorPMA, texture(g_texture0, uv));
}
)glsl";

	//
	//	Line patterns (uv.x is the distance along the line)
	//
	constexpr std::string_view GLSL_PS_LineDot =
R"glsl(void main()
{
	vec4 result = v_colorPMA;
	float u = (0.5 * (v_uv.x - 0.5));
	float w = fwidth(u);
	float value = abs(2.0 * fract(u) - 1.0);
	result *= smoothstep((0.5 - w), (0.5 + w), value);
	o_color = s3d_shapeColor(result);
}
)glsl";

	constexpr std::string_view GLSL_PS_LineDash =
R"glsl(void main()
{
	vec4 result = v_colorPMA;
	float u = (0.25 * (v_uv.x - 1.0));
	float w = fwidth(u);
	float dist = abs(2.0 * fract(u) - 1.0);
	result *= smoothstep((0.4 - w), (0.4 + w), dist);
	o_color = s3d_shapeColor(result);
}
)glsl";

	constexpr std::string_view GLSL_PS_LineLongDash =
R"glsl(void main()
{
	vec4 result = v_colorPMA;
	float u = (0.1 * (v_uv.x - 1.0));
	float w = fwidth(u);
	float dist = abs(2.0 * fract(u) - 1.0);
	result *= smoothstep((0.3 - w), (0.3 + w), dist);
	o_color = s3d_shapeColor(result);
}
)glsl";

	constexpr std::string_view GLSL_PS_LineDashDot =
R"glsl(void main()
{
	vec4 result = v_colorPMA;
	float u = (0.1 * (v_uv.x - 1.0));
	float u2 = (u + 0.5);
	float w = fwidth(u);
	float dist = abs(2.0 * fract(u) - 1.0);
	float dist2 = abs(2.0 * fract(u2) - 1.0);
	float a1 = smoothstep((0.4 - w), (0.4 + w), dist);
	float a2 = smoothstep((0.9 - w), (0.9 + w), dist2);
	result *= max(a1, a2);
	o_color = s3d_shapeColor(result);
}
)glsl";

	constexpr std::string_view GLSL_PS_LineRoundDot =
R"glsl(void main()
{
	vec4 result = v_colorPMA;
	vec2 uv = ((v_uv + vec2(0.5, 0.0)) * vec2(0.5, 1.0));
	float w = fwidth(uv.y);
	float dist = length(vec2(4.0, 2.0) * fract(uv) - vec2(2.0, 1.0));
	result *= (1.0 - smoothstep((1.0 - w), (1.0 + w), dist));
	o_color = s3d_shapeColor(result);
}
)glsl";

	//
	//	Fill patterns
	//
	constexpr std::string_view GLSL_PS_PatternPolkaDot =
R"glsl(void main()
{
	vec2 uv = Pattern_UVTransform(gl_FragCoord.xy);
	vec2 repeat = (2.0 * fract(uv) - 1.0);
	float value = length(repeat);
	float fw = (length(vec2(dFdx(value), dFdy(value))) * 0.70710678118);
	float radiusScale = g_patternUVTransform[1].z;
	float c = smoothstep((radiusScale - fw), (radiusScale + fw), value);
	o_color = mix(s3d_shapeColor(v_colorPMA), Pattern_BackgroundColor(), c);
}
)glsl";

	constexpr std::string_view GLSL_PS_PatternStripe =
R"glsl(void main()
{
	float u = Pattern_UVTransform(gl_FragCoord.xy).x;
	float fw = fwidth(u);
	float value = abs(2.0 * fract(u) - 1.0);
	float thicknessScale = (g_patternUVTransform[1].z * (1.0 + 2.0 * fw) - fw);
	float c = smoothstep((thicknessScale - fw), (thicknessScale + fw), value);
	o_color = mix(s3d_shapeColor(v_colorPMA), Pattern_BackgroundColor(), c);
}
)glsl";

	constexpr std::string_view GLSL_PS_PatternGrid =
R"glsl(void main()
{
	vec2 uv = Pattern_UVTransform(gl_FragCoord.xy);
	vec2 fw = fwidth(uv);
	vec2 value = abs(2.0 * fract(uv) - 1.0);
	vec2 thicknessScale = (vec2(g_patternUVTransform[1].z) * (1.0 + fw) - fw);
	vec2 c = smoothstep((thicknessScale - fw), (thicknessScale + fw), value);
	o_color = mix(s3d_shapeColor(v_colorPMA), Pattern_BackgroundColor(), min(c.x, c.y));
}
)glsl";

	constexpr std::string_view GLSL_PS_PatternChecker =
R"glsl(void main()
{
	vec2 uv = Pattern_UVTransform(gl_FragCoord.xy);
	float c = Pattern_CheckersFiltered(uv, g_patternUVTransform[1].zw);
	o_color = mix(s3d_shapeColor(v_colorPMA), Pattern_BackgroundColor(), c);
}
)glsl";

	constexpr std::string_view GLSL_PS_PatternTriangle =
R"glsl(void main()
{
	vec2 uv = Pattern_UVTransform(gl_FragCoord.xy);
	vec2 fw = (fwidth(uv) * 0.25);
	vec2 s1 = Pattern_Skew(uv + vec2(-fw.x, -fw.y));
	vec2 s2 = Pattern_Skew(uv + vec2( fw.x,  fw.y));
	vec2 s3 = Pattern_Skew(uv + vec2(-fw.x,  fw.y));
	vec2 s4 = Pattern_Skew(uv + vec2( fw.x, -fw.y));
	vec4 f1 = fract(vec4(s1, s2));
	vec4 f2 = fract(vec4(s3, s4));
	vec4 ss = vec4(step(f1.x, f1.y), step(f1.z, f1.w), step(f2.x, f2.y), step(f2.z, f2.w));
	float c = dot(ss, vec4(0.25));
	o_color = mix(s3d_shapeColor(v_colorPMA), Pattern_BackgroundColor(), c);
}
)glsl";

	constexpr std::string_view GLSL_PS_PatternHexGrid =
R"glsl(void main()
{
	vec2 uv = Pattern_UVTransform(gl_FragCoord.xy);
	vec2 fw = fwidth(uv);
	float w = (max(fw.x, fw.y) * 0.5);
	float thicknessScale = (g_patternUVTransform[1].z * (1.0 + 2.0 * w));
	float c = smoothstep((thicknessScale - w), (thicknessScale + w), Pattern_Hex(uv));
	o_color = mix(Pattern_BackgroundColor(), s3d_shapeColor(v_colorPMA), c);
}
)glsl";

	//
	//	MSDF font (median distance + screen-space AA via texture dimensions)
	//
	constexpr std::string_view GLSL_PSMSDFPrefix =
R"glsl(const float MSDF_PixelRange = 16.0;
const float MSDF_TextThreshold = 0.5;
float MSDF_Median(vec3 c) { return max(min(c.r, c.g), min(max(c.r, c.g), c.b)); }
float MSDF_SampleDistance(vec2 uv) { return MSDF_Median(texture(g_texture0, uv).rgb); }
float MSDF_Coverage(float d, float threshold, float scale) { return clamp((d - threshold) * scale + 0.5, 0.0, 1.0); }
vec2 MSDF_InvTextureSize() { return (1.0 / vec2(textureSize(g_texture0, 0))); }
float MSDF_Scale()
{
	vec2 invTexSize = MSDF_InvTextureSize();
	vec2 msdfUnit = (MSDF_PixelRange * invTexSize);
	vec2 screenPixelRange = (0.5 / fwidth(v_uv));
	return dot(msdfUnit, screenPixelRange);
}
vec4 MSDF_PremulAdd(vec4 c) { return (c + (g_colorAdd * c.a)); }
)glsl";

	constexpr std::string_view GLSL_PS_FontMSDF =
R"glsl(void main()
{
	float textAlpha = MSDF_Coverage(MSDF_SampleDistance(v_uv), MSDF_TextThreshold, MSDF_Scale());
	o_color = MSDF_PremulAdd(v_colorPMA * textAlpha);
}
)glsl";

	constexpr std::string_view GLSL_PS_FontMSDF_Outline =
R"glsl(void main()
{
	float scale = MSDF_Scale();
	float d = MSDF_SampleDistance(v_uv);
	float outlineAlpha = MSDF_Coverage(d, g_sdfParam.y, scale);
	float textAlpha = MSDF_Coverage(d, g_sdfParam.x, scale);
	vec4 colorPMA = mix(g_sdfOutlineColorPMA, v_colorPMA, textAlpha);
	colorPMA *= outlineAlpha;
	o_color = MSDF_PremulAdd(colorPMA);
}
)glsl";

	constexpr std::string_view GLSL_PS_FontMSDF_Shadow =
R"glsl(void main()
{
	float scale = MSDF_Scale();
	float textAlpha = MSDF_Coverage(MSDF_SampleDistance(v_uv), MSDF_TextThreshold, scale);
	vec2 shadowOffset = (g_sdfParam.zw * MSDF_InvTextureSize());
	float shadowAlpha = MSDF_Coverage(MSDF_SampleDistance(v_uv - shadowOffset), MSDF_TextThreshold, scale);
	float sBase = (shadowAlpha * (1.0 - textAlpha));
	vec4 finalPMA = ((v_colorPMA * textAlpha) + (g_sdfShadowColorPMA * sBase));
	o_color = MSDF_PremulAdd(finalPMA);
}
)glsl";

	constexpr std::string_view GLSL_PS_FontMSDF_OutlineShadow =
R"glsl(void main()
{
	float scale = MSDF_Scale();
	float d = MSDF_SampleDistance(v_uv);
	float outlineAlpha = MSDF_Coverage(d, g_sdfParam.y, scale);
	float textAlpha = MSDF_Coverage(d, g_sdfParam.x, scale);
	vec2 shadowOffset = (g_sdfParam.zw * MSDF_InvTextureSize());
	float shadowAlpha = MSDF_Coverage(MSDF_SampleDistance(v_uv - shadowOffset), g_sdfParam.y, scale);
	vec4 textPMA = (v_colorPMA * textAlpha);
	float outlineCoverage = clamp(outlineAlpha - textAlpha, 0.0, 1.0);
	vec4 outlinePMA = (g_sdfOutlineColorPMA * outlineCoverage);
	float shadowCoverage = clamp(shadowAlpha * (1.0 - outlineAlpha), 0.0, 1.0);
	vec4 shadowPMA = (g_sdfShadowColorPMA * shadowCoverage);
	o_color = MSDF_PremulAdd(textPMA + outlinePMA + shadowPMA);
}
)glsl";

	constexpr std::string_view GLSL_PS_FontMSDF_Glow =
R"glsl(void main()
{
	float d = clamp(texture(g_texture0, v_uv).a * 2.0, 0.0, 1.0);
	float pd = pow(abs(d), g_sdfParam.x);
	vec4 finalPMA = vec4((v_colorPMA.rgb * pd), (v_colorPMA.a * pd));
	o_color = MSDF_PremulAdd(finalPMA);
}
)glsl";

	constexpr std::string_view GLSL_PS_FontPrint =
R"glsl(void main()
{
	float scale = MSDF_Scale();
	float d = MSDF_SampleDistance(v_uv);
	float outlineThreshold = (MSDF_TextThreshold - 0.04);
	float textAlpha = sqrt(clamp((d - 0.5) * scale + 0.5, 0.0, 1.0));
	float outlineAlpha = sqrt(clamp((d - outlineThreshold) * scale + 0.5, 0.0, 1.0));
	vec2 shadowOffset = (vec2(0.625) * MSDF_InvTextureSize());
	float d2 = MSDF_SampleDistance(v_uv - shadowOffset);
	float shadowAlpha = sqrt(clamp((d2 - outlineThreshold) * scale + 0.5, 0.0, 1.0));
	vec3 color = mix(vec3(0.0), vec3(1.0), textAlpha);
	float hollowShadowAlpha = clamp(shadowAlpha * (1.0 - outlineAlpha), 0.0, 1.0);
	color = mix(color, vec3(0.0), hollowShadowAlpha);
	float finalAlpha = clamp(outlineAlpha + hollowShadowAlpha, 0.0, 1.0);
	o_color = vec4(color, finalAlpha);
}
)glsl";

	constexpr std::string_view GLSL_PS_FullScreenTriangle =
R"glsl(#version 410 core
layout(location = 0) in vec2 v_uv;
out vec4 o_color;
uniform sampler2D g_texture0;
void main()
{
	o_color = vec4(texture(g_texture0, v_uv).rgb, 1.0);
}
)glsl";
}
