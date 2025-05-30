﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2025 Ryo Suzuki
//	Copyright (c) 2016-2025 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D/EnvironmentVariable.hpp>
# include <Siv3D/Unicode.hpp>

namespace s3d
{
	namespace EnvironmentVariable
	{
		////////////////////////////////////////////////////////////////
		//
		//	Get
		//
		////////////////////////////////////////////////////////////////

		String Get(const StringView name)
		{
			wchar_t* pValue;
			size_t len;
			errno_t err = ::_wdupenv_s(&pValue, &len, Unicode::ToWstring(name).c_str());

			if (err || (not pValue))
			{
				return{};
			}

			String value = Unicode::FromWstring(std::wstring_view{ pValue, len });

			std::free(pValue);

			return value;
		}
	}
}
