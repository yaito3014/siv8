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

# include <sys/stat.h>
# include <unistd.h>
# include <ctime>
# include <cstdlib>
# include <filesystem>
# include <fstream>
# include <Siv3D/Array.hpp>
# include <Siv3D/ArrayAlgorithm.hpp>
# include <Siv3D/String.hpp>
# include <Siv3D/StringView.hpp>
# include <Siv3D/FileSystem.hpp>
# include <Siv3D/Unicode.hpp>
# include <Siv3D/SpecialFolder.hpp>
# include <Siv3D/EnvironmentVariable.hpp>
# include "LinuxFileSystem.hpp"

namespace s3d
{
	namespace detail
	{
		namespace init
		{
			const static FilePathCache g_filePathCache{};

			const Array<FilePath>& GetResourceFiles() noexcept
			{
				return g_filePathCache.resourceFilePaths;
			}
		}

		[[nodiscard]]
		static std::filesystem::path ToPath(const FilePathView path)
		{
			return std::filesystem::path(Unicode::ToUTF8(path));
		}

		[[nodiscard]]
		static bool GetStat(const FilePathView path, struct stat& s)
		{
			return (::stat(Unicode::ToUTF8(FilePath{ path }.replaced(U'\\', U'/')).c_str(), &s) == 0);
		}

		[[nodiscard]]
		static bool Exists(const FilePathView path)
		{
			struct stat s;
			return GetStat(path, s);
		}

		[[nodiscard]]
		static bool IsRegular(const FilePathView path)
		{
			struct stat s;
			if (!GetStat(path, s))
			{
				return false;
			}

			return S_ISREG(s.st_mode);
		}

		[[nodiscard]]
		static bool IsDirectory(const FilePathView path)
		{
			struct stat s;
			if (!GetStat(path, s))
			{
				return false;
			}

			return S_ISDIR(s.st_mode);
		}

		[[nodiscard]]
		static FilePath ParentDirectoryOf(const FilePathView path)
		{
			FilePath parent = Unicode::FromUTF8(ToPath(path).parent_path().string());

			if (parent && (not parent.ends_with(U'/')))
			{
				parent.push_back(U'/');
			}

			return parent;
		}

		// freedesktop.org トラッシュ仕様（ホームトラッシュのみ対応）に基づきファイルを移動する。
		[[nodiscard]]
		static bool Linux_TrashFile(const FilePathView path)
		{
			const FilePath dataHome = []() -> FilePath
			{
				if (const FilePath xdg = EnvironmentVariable::Get(U"XDG_DATA_HOME"))
				{
					return xdg;
				}

				if (const FilePath home = EnvironmentVariable::Get(U"HOME"))
				{
					return (home + U"/.local/share");
				}

				return{};
			}();

			if (not dataHome)
			{
				return false;
			}

			const std::filesystem::path trashFiles = ToPath(dataHome + U"/Trash/files");
			const std::filesystem::path trashInfo = ToPath(dataHome + U"/Trash/info");

			std::error_code ec;
			std::filesystem::create_directories(trashFiles, ec);
			std::filesystem::create_directories(trashInfo, ec);

			const std::filesystem::path source = ToPath(path);
			std::string name = source.filename().string();

			// 同名衝突の回避。
			std::filesystem::path dest = (trashFiles / name);
			std::filesystem::path info = (trashInfo / (name + ".trashinfo"));
			for (int32 i = 1; (std::filesystem::exists(dest, ec) || std::filesystem::exists(info, ec)); ++i)
			{
				const std::string candidate = (source.stem().string() + "_" + std::to_string(i) + source.extension().string());
				dest = (trashFiles / candidate);
				info = (trashInfo / (candidate + ".trashinfo"));
			}

			char deletionDate[32] = {};
			{
				const std::time_t now = std::time(nullptr);
				std::tm lt;
				::localtime_r(&now, &lt);
				std::strftime(deletionDate, sizeof(deletionDate), "%Y-%m-%dT%H:%M:%S", &lt);
			}

			std::filesystem::rename(source, dest, ec);

			if (ec) // 別ファイルシステム等で rename が失敗した場合はコピー後に削除。
			{
				ec.clear();
				std::filesystem::copy(source, dest, (std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing), ec);

				if (ec)
				{
					return false;
				}

				std::filesystem::remove_all(source, ec);
			}

			std::ofstream infoFile{ info };
			if (infoFile)
			{
				infoFile << "[Trash Info]\n"
						 << "Path=" << std::filesystem::absolute(source).string() << "\n"
						 << "DeletionDate=" << deletionDate << "\n";
			}

			return true;
		}
	}

	namespace detail
	{
		FilePathCache::FilePathCache()
		{
			// 実行ファイルのフルパス（/proc/self/exe）。
			executablePath = []() -> FilePath
			{
				char buffer[4096];
				const ssize_t len = ::readlink("/proc/self/exe", buffer, (sizeof(buffer) - 1));

				if (len == -1)
				{
					return{};
				}

				buffer[len] = '\0';
				return Unicode::FromUTF8(std::string(buffer, static_cast<size_t>(len)));
			}();

			// 実行ファイルのあるディレクトリ。
			executableDirectory = ParentDirectoryOf(executablePath);

			// 起動時のカレントディレクトリ。
			launchDirectory = []() -> FilePath
			{
				std::error_code ec;
				FilePath dir = Unicode::FromUTF8(std::filesystem::current_path(ec).string());

				if (dir && (not dir.ends_with(U'/')))
				{
					dir.push_back(U'/');
				}

				return dir;
			}();

			// 特殊フォルダ。
			specialFolderPaths.fill(FilePath{});
			specialFolderPaths[FromEnum(SpecialFolder::ProgramFiles)]	= U"/usr/";
			specialFolderPaths[FromEnum(SpecialFolder::LocalAppData)]	= U"/var/cache/";
			specialFolderPaths[FromEnum(SpecialFolder::SystemFonts)]	= U"/usr/share/fonts/";
			specialFolderPaths[FromEnum(SpecialFolder::LocalFonts)]		= U"/usr/local/share/fonts/";

			const FilePath homeDirectory = EnvironmentVariable::Get(U"HOME");

			if (homeDirectory)
			{
				if (const FilePath cacheHome = EnvironmentVariable::Get(U"XDG_CACHE_HOME"))
				{
					specialFolderPaths[FromEnum(SpecialFolder::LocalAppData)] = (cacheHome + U'/');
				}
				else
				{
					specialFolderPaths[FromEnum(SpecialFolder::LocalAppData)] = (homeDirectory + U"/.cache/");
				}

				specialFolderPaths[FromEnum(SpecialFolder::UserFonts)]		= (homeDirectory + U"/.local/share/fonts/");
				specialFolderPaths[FromEnum(SpecialFolder::UserProfile)]	= (homeDirectory + U'/');

				// user-dirs.dirs から XDG ユーザーディレクトリを取得する。
				const FilePath iniFilePath = (homeDirectory + U"/.config/user-dirs.dirs");

				std::ifstream ini{ Unicode::ToUTF8(iniFilePath) };

				if (ini)
				{
					const auto assign = [&](const std::string& key, const SpecialFolder folder)
					{
						return [&, key, folder](const std::string& line)
						{
							if (not line.starts_with(key + "="))
							{
								return false;
							}

							std::string value = line.substr(key.size() + 1);

							// 引用符を除去。
							if ((2 <= value.size()) && (value.front() == '"') && (value.back() == '"'))
							{
								value = value.substr(1, (value.size() - 2));
							}

							// $HOME の展開。
							if (value.starts_with("$HOME"))
							{
								value = (Unicode::ToUTF8(homeDirectory) + value.substr(5));
							}

							FilePath result = Unicode::FromUTF8(value);

							if (result && (not result.ends_with(U'/')))
							{
								result.push_back(U'/');
							}

							specialFolderPaths[FromEnum(folder)] = result;
							return true;
						};
					};

					const auto handlers = {
						assign("XDG_DESKTOP_DIR",	SpecialFolder::Desktop),
						assign("XDG_DOCUMENTS_DIR",	SpecialFolder::Documents),
						assign("XDG_DOWNLOAD_DIR",	SpecialFolder::Downloads),
						assign("XDG_MUSIC_DIR",		SpecialFolder::Music),
						assign("XDG_PICTURES_DIR",	SpecialFolder::Pictures),
						assign("XDG_VIDEOS_DIR",	SpecialFolder::Videos),
					};

					std::string line;
					while (std::getline(ini, line))
					{
						if (line.empty() || (line.front() == '#'))
						{
							continue;
						}

						for (const auto& handler : handlers)
						{
							if (handler(line))
							{
								break;
							}
						}
					}
				}
			}

			// エンジンリソース（実行ファイルの隣の engine/ ディレクトリ）。
			resourceFilePaths = [this]()
			{
				const FilePath resourcePath = (executableDirectory + U"engine/");

				Array<FilePath> paths = FileSystem::DirectoryContents(resourcePath, Recursive::Yes);

				paths.erase_all_if(FileSystem::IsDirectory);

				paths.sort();

				return paths;
			}();
		}
	}

	namespace FileSystem
	{
		////////////////////////////////////////////////////////////////
		//
		//	IsResourcePath
		//
		////////////////////////////////////////////////////////////////

		bool IsResourcePath(const FilePathView path) noexcept
		{
			const FilePath resourceDirectory = (detail::init::g_filePathCache.executableDirectory + U"engine/");
			return FullPath(path).starts_with(resourceDirectory);
		}

		////////////////////////////////////////////////////////////////
		//
		//	Exists
		//
		////////////////////////////////////////////////////////////////

		bool Exists(const FilePathView path)
		{
			if (path.isEmpty())
			{
				return false;
			}

			return detail::Exists(path);
		}

		////////////////////////////////////////////////////////////////
		//
		//	IsDirectory
		//
		////////////////////////////////////////////////////////////////

		bool IsDirectory(const FilePathView path)
		{
			if (path.isEmpty())
			{
				return false;
			}

			return detail::IsDirectory(path);
		}

		////////////////////////////////////////////////////////////////
		//
		//	IsFile
		//
		////////////////////////////////////////////////////////////////

		bool IsFile(const FilePathView path)
		{
			if (path.isEmpty())
			{
				return false;
			}

			return detail::IsRegular(path);
		}

		////////////////////////////////////////////////////////////////
		//
		//	IsResource
		//
		////////////////////////////////////////////////////////////////

		bool IsResource(const FilePathView path)
		{
			return (IsResourcePath(path) && detail::Exists(path));
		}

		////////////////////////////////////////////////////////////////
		//
		//	NativePath
		//
		////////////////////////////////////////////////////////////////

		NativeFilePath NativePath(const FilePathView path)
		{
			if (not path)
			{
				return{};
			}

			std::error_code ec;
			return std::filesystem::weakly_canonical(detail::ToPath(path), ec).string();
		}

		////////////////////////////////////////////////////////////////
		//
		//	VolumePath
		//
		////////////////////////////////////////////////////////////////

		FilePath VolumePath(const FilePathView)
		{
			return U"/";
		}

		////////////////////////////////////////////////////////////////
		//
		//	DirectoryContents
		//
		////////////////////////////////////////////////////////////////

		Array<FilePath> DirectoryContents(const FilePathView path, const Recursive recursive)
		{
			Array<FilePath> paths;

			if (path.isEmpty() || !IsDirectory(path))
			{
				return paths;
			}

			if (recursive)
			{
				for (const auto& v : std::filesystem::recursive_directory_iterator{ Unicode::ToUTF8(path) })
				{
					paths.push_back(FullPath(Unicode::FromUTF8(v.path().string())));
				}
			}
			else
			{
				for (const auto& v : std::filesystem::directory_iterator{ Unicode::ToUTF8(path) })
				{
					paths.push_back(FullPath(Unicode::FromUTF8(v.path().string())));
				}
			}

			return paths;
		}

		////////////////////////////////////////////////////////////////
		//
		//	GetLaunchDirectory
		//
		////////////////////////////////////////////////////////////////

		const FilePath& GetLaunchDirectory() noexcept
		{
			return detail::init::g_filePathCache.launchDirectory;
		}

		////////////////////////////////////////////////////////////////
		//
		//	GetExecutablePath
		//
		////////////////////////////////////////////////////////////////

		const FilePath& GetExecutablePath() noexcept
		{
			return detail::init::g_filePathCache.executablePath;
		}

		////////////////////////////////////////////////////////////////
		//
		//	GetExecutableDirectory
		//
		////////////////////////////////////////////////////////////////

		const FilePath& GetExecutableDirectory() noexcept
		{
			return detail::init::g_filePathCache.executableDirectory;
		}

		////////////////////////////////////////////////////////////////
		//
		//	ChangeCurrentDirectory
		//
		////////////////////////////////////////////////////////////////

		bool ChangeCurrentDirectory(const FilePathView path)
		{
			if (not IsDirectory(path))
			{
				return false;
			}

			return (::chdir(Unicode::ToUTF8(path).c_str()) == 0);
		}

		////////////////////////////////////////////////////////////////
		//
		//	GetFolderPath
		//
		////////////////////////////////////////////////////////////////

		const FilePath& GetFolderPath(const SpecialFolder folder)
		{
			assert(FromEnum(folder) < static_cast<int32>(std::size(detail::init::g_filePathCache.specialFolderPaths)));

			return detail::init::g_filePathCache.specialFolderPaths[FromEnum(folder)];
		}

		////////////////////////////////////////////////////////////////
		//
		//	Remove
		//
		////////////////////////////////////////////////////////////////

		bool Remove(const FilePathView path, const MoveToTrash moveToTrash)
		{
			if (not path)
			{
				return false;
			}

			if (IsResourcePath(path))
			{
				return false;
			}

			if (moveToTrash)
			{
				return detail::Linux_TrashFile(path);
			}
			else
			{
				try
				{
					std::filesystem::remove_all(detail::ToPath(path));
					return true;
				}
				catch (const std::filesystem::filesystem_error&)
				{
					return false;
				}
			}
		}
	}
}
