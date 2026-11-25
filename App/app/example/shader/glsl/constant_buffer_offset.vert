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

// Custom constant buffer: VS slot 2 holds Float4 offset (xy in local drawing
// coordinates). GLSL counterpart of VS_Offset in hlsl/constant_buffer.hlsl.

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) in vec2 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec4 i_color;

layout(location = 0) out vec4 v_colorPMA;
layout(location = 1) out vec2 v_uv;

layout(std140) uniform VSConstants2D
{
	vec4 g_transform[3];
	vec4 g_colorMul;
};

layout(std140, binding = 2) uniform VSOffset
{
	vec4 g_offset;
};

void main()
{
	const vec2 position = (i_position + g_offset.xy);
	const vec4 clip = ((position.x * g_transform[0]) + (position.y * g_transform[1]) + g_transform[2]);
	gl_Position = vec4(clip.xy, 0.0, clip.w);

	const vec4 color = (i_color * g_colorMul);
	v_colorPMA = vec4((color.rgb * color.a), color.a);
	v_uv = i_uv;
}
