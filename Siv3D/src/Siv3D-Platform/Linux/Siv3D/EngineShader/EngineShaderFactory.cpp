//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "GL4/CEngineShader_GL4.hpp"

namespace s3d
{
	ISiv3DEngineShader* ISiv3DEngineShader::Create()
	{
		return new CEngineShader_GL4;
	}
}
