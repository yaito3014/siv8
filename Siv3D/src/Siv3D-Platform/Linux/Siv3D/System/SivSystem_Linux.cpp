//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <unistd.h>
# include <sys/wait.h>
# include <Siv3D/System.hpp>
# include <Siv3D/Unicode.hpp>
# include <Siv3D/FileSystem.hpp>

namespace s3d
{
	namespace System
	{
		bool OpenInBrowser(const URLView url)
		{
			// Web URL か、ローカルの HTML ファイルのみを許可する（macOS 実装に準拠）。
			if (not (url.starts_with(U"http://") || url.starts_with(U"https://")))
			{
				const String extension = FileSystem::Extension(url);

				if ((extension != U"html") && (extension != U"htm"))
				{
					return false;
				}
			}

			const std::string target = Unicode::ToUTF8(url);

			const pid_t pid = ::fork();

			if (pid < 0)
			{
				return false;
			}

			if (pid == 0)
			{
				::execlp("xdg-open", "xdg-open", target.c_str(), static_cast<char*>(nullptr));
				::_exit(127); // exec 失敗。
			}

			int status = 0;

			if (::waitpid(pid, &status, 0) < 0)
			{
				return false;
			}

			return (WIFEXITED(status) && (WEXITSTATUS(status) == 0));
		}
	}
}
