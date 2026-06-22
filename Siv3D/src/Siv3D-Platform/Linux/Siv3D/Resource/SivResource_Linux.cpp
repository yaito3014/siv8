//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D/Resource.hpp>
# include <Siv3D/FileSystem.hpp>

namespace s3d
{
	namespace detail::init
	{
		[[nodiscard]]
		const Array<FilePath>& GetResourceFiles() noexcept;
	}

	const Array<FilePath>& EnumResourceFiles() noexcept
	{
		return detail::init::GetResourceFiles();
	}

	FilePath Resource(const FilePathView path)
	{
		// Linux にアプリケーションバンドルは無いので、実行ファイル隣の resources/ をリソースルートとする。
		const FilePath resourceDirectory = (FileSystem::GetExecutableDirectory() + U"resources/");

		return (resourceDirectory + path);
	}
}
