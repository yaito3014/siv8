//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2026 Ryo Suzuki
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

#version 410 core
#extension GL_ARB_shading_language_420pack : require

// Custom constant buffer: PS slot 2 holds Float4 tint (straight RGBA).
// GLSL counterpart of PS_TintTexture in hlsl/constant_buffer.hlsl.

layout(location = 0) in vec4 v_colorPMA;
layout(location = 1) in vec2 v_uv;

layout(location = 0) out vec4 o_color;

uniform sampler2D g_texture0;

layout(std140) uniform PSConstants2D
{
	vec4 g_patternBackgroundColorMul;
	vec4 g_colorAdd;
	vec4 g_sdfParam;
	vec4 g_sdfOutlineColorPMA;
	vec4 g_sdfShadowColorPMA;
};

layout(std140, binding = 2) uniform PSTint
{
	vec4 g_tint;
};

void main()
{
	vec4 colorPMA = (v_colorPMA * texture(g_texture0, v_uv));
	colorPMA += (g_colorAdd * colorPMA.a);
	o_color = (colorPMA * vec4((g_tint.rgb * g_tint.a), g_tint.a));
}
