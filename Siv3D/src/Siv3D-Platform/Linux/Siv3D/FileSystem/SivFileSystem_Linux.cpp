//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2023 Ryo Suzuki
//	Copyright (c) 2016-2023 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <sys/stat.h>
# include <unistd.h>
# include <glib-2.0/glib.h>
# include <glib-2.0/gio/gio.h>
# include <filesystem>
# include <Siv3D/String.hpp>
# include <Siv3D/FileSystem.hpp>
# include <Siv3D/EnvironmentVariable.hpp>
# include <Siv3D/SpecialFolder.hpp>
# include <Siv3D/INI.hpp>

namespace s3d
{
  namespace fs = std::filesystem;

  namespace detail
  {
    [[nodiscard]]
    static fs::path ToPath(const FilePathView path)
    {
      return fs::path(path.toUTF8());
    }

    [[nodiscard]]
    static fs::file_status GetStatus(const FilePathView path)
    {
      return fs::status(detail::ToPath(path));
    }

    [[nodiscard]]
    static bool GetStat(const FilePathView path, struct stat& s)
    {
      return (::stat(Unicode::ToUTF8(FilePath(path).replaced(U'\\', U'/')).c_str(), &s) == 0);
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
    static FilePath NormalizePath(FilePath path, const bool skipDirectoryCheck = false)
    {
      path.replace(U'\\', U'/');

      if (!path.ends_with(U'/')
        && (skipDirectoryCheck || (GetStatus(path).type() == fs::file_type::directory)))
      {
        path.push_back(U'/');
      }

      return path;
    }

    [[nodiscard]]
    static DateTime ToDateTime(const ::timespec& tv)
    {
      ::tm lt;
      ::localtime_r(&tv.tv_sec, &lt);
      return{ (1900 + lt.tm_year),
          (1 + lt.tm_mon),
          (lt.tm_mday),
          lt.tm_hour,
          lt.tm_min,
          lt.tm_sec,
          static_cast<int32>(tv.tv_nsec / (1'000'000)) };
    }

    [[nodiscard]]
    inline constexpr std::filesystem::copy_options ToCopyOptions(const CopyOption copyOption) noexcept
    {
      switch (copyOption)
      {
      case CopyOption::SkipExisting:
        return fs::copy_options::skip_existing;
      case CopyOption::OverwriteExisting:
        return fs::copy_options::overwrite_existing;
      case CopyOption::UpdateExisting:
        return fs::copy_options::update_existing;
      default:
        return fs::copy_options::none;
      }
    }

    [[nodiscard]]
    static bool Linux_TrashFile(const char* path)
    {
      GFile* gf = g_file_new_for_path(path);
      GError* ge;

      gboolean ret = g_file_trash(gf, nullptr, &ge);

      if (ge)
      {
        g_error_free(ge);
      }

      g_object_unref(gf);

      if (!ret)
      {
        return false;
      }

      return true;
    }

    [[nodiscard]]
    FilePath AddSlash(FilePath path)
    {
      if (path.ends_with(U'/'))
      {
        return path;
      }

      return path + U'/';
    }

    namespace init
    {
      const static FilePath g_initialPath = NormalizePath(Unicode::FromUTF8(fs::current_path().string()));

      static FilePath g_modulePath;

      void InitModulePath(const char* arg)
      {
        g_modulePath = Unicode::FromUTF8(arg);
      }

      const static std::array<FilePath, 11> g_specialFolderPaths = []()
        {
          std::array<FilePath, 11> specialFolderPaths;

          specialFolderPaths[FromEnum(SpecialFolder::ProgramFiles)] = U"/usr/";
          specialFolderPaths[FromEnum(SpecialFolder::LocalAppData)] = U"/var/cache/";
          specialFolderPaths[FromEnum(SpecialFolder::SystemFonts)] = U"/usr/share/fonts/";

          const FilePath homeDirectory = EnvironmentVariable::Get(U"HOME");

          if (not homeDirectory)
          {
            return specialFolderPaths;
          }

          if (const FilePath localCacheDirectory = EnvironmentVariable::Get(U"XDG_CACHE_HOME"))
          {
            specialFolderPaths[FromEnum(SpecialFolder::LocalAppData)] = (localCacheDirectory + U'/');
          }
          else
          {
            specialFolderPaths[FromEnum(SpecialFolder::LocalAppData)] = (homeDirectory + U"/.cache/");
          }

          if (const FilePath localFontDirectory = homeDirectory + U"/usr/local/share/fonts/";
            FileSystem::Exists(localFontDirectory))
          {
            specialFolderPaths[FromEnum(SpecialFolder::LocalFonts)] = localFontDirectory;
            specialFolderPaths[FromEnum(SpecialFolder::UserFonts)] = localFontDirectory;
          }

          specialFolderPaths[FromEnum(SpecialFolder::UserProfile)] = (homeDirectory + U'/');

          const FilePath iniFilePath = homeDirectory + U"/.config/user-dirs.dirs";

          if (not FileSystem::Exists(iniFilePath))
          {
            return specialFolderPaths;
          }

          const INI ini(iniFilePath);
          specialFolderPaths[FromEnum(SpecialFolder::Desktop)] = AddSlash(ini[U"XDG_DESKTOP_DIR"].name.removed(U'\"').replaced(U"$HOME", homeDirectory));
          specialFolderPaths[FromEnum(SpecialFolder::Documents)] = AddSlash(ini[U"XDG_DOCUMENTS_DIR"].name.removed(U'\"').replaced(U"$HOME", homeDirectory));
          specialFolderPaths[FromEnum(SpecialFolder::Music)] = AddSlash(ini[U"XDG_MUSIC_DIR"].name.removed(U'\"').replaced(U"$HOME", homeDirectory));
          specialFolderPaths[FromEnum(SpecialFolder::Pictures)] = AddSlash(ini[U"XDG_PICTURES_DIR"].name.removed(U'\"').replaced(U"$HOME", homeDirectory));
          specialFolderPaths[FromEnum(SpecialFolder::Videos)] = AddSlash(ini[U"XDG_VIDEOS_DIR"].name.removed(U'\"').replaced(U"$HOME", homeDirectory));

          return specialFolderPaths;
        }();

      const static Array<FilePath> g_resourceFilePaths = []()
        {
          Array<FilePath> paths = FileSystem::DirectoryContents(U"resources/", Recursive::Yes);

          paths.remove_if(FileSystem::IsDirectory);

          paths.sort();

          return paths;
        }();

      const Array<FilePath>& GetResourceFilePaths() noexcept
      {
        return g_resourceFilePaths;
      }
    }
  }

  namespace FileSystem
  {
    bool IsResourcePath(const FilePathView path) noexcept
    {
      const FilePath resourceDirectory = FileSystem::ModulePath() + U"/resources/";

      return FullPath(path).starts_with(resourceDirectory);
    }

    bool Exists(const FilePathView path)
    {
      if (not path) {
        return false;
      }

      return detail::Exists(path);
    }

    bool IsDirectory(const FilePathView path)
    {
      if (not path)
      {
        return false;
      }

      return detail::IsDirectory(path);
    }

    bool IsFile(const FilePathView path)
    {
      if (not path) {
        return false;
      }

      return detail::IsRegular(path);
    }

    bool IsResource(const FilePathView path)
    {
      return IsResourcePath(path)
        && detail::Exists(path);
    }

    NativeFilePath NativePath(const FilePathView path)
    {
      if (not path) {
        return NativeFilePath{};
      }

      return fs::weakly_canonical(detail::ToPath(path)).string();
    }

    FilePath VolumePath(const FilePathView)
    {
      return U"/";
    }

    Array<FilePath> DirectoryContents(const FilePathView path, const Recursive recursive)
    {
      Array<FilePath> paths;

      if (not path) {
        return paths;
      }

      if (detail::GetStatus(path).type() != fs::file_type::directory)
      {
        return paths;
      }

      if (recursive)
      {
        for (const auto& v : fs::recursive_directory_iterator(Unicode::ToUTF8(path)))
        {
          paths.push_back(detail::NormalizePath(Unicode::FromUTF8(fs::weakly_canonical(v.path()).string())));
        }
      }
      else
      {
        for (const auto& v : fs::directory_iterator(Unicode::ToUTF8(path)))
        {
          paths.push_back(detail::NormalizePath(Unicode::FromUTF8(fs::weakly_canonical(v.path()).string())));
        }
      }

      return paths;
    }

    const FilePath& InitialDirectory() noexcept
    {
      return detail::init::g_initialPath;
    }

    const FilePath& ModulePath() noexcept
    {
      return detail::init::g_modulePath;
    }

    bool ChangeCurrentDirectory(const FilePathView path)
    {
      if (not IsDirectory(path))
      {
        return false;
      }

      return (chdir(Unicode::ToUTF8(path).c_str()) == 0);
    }

    const FilePath& GetFolderPath(const SpecialFolder folder)
    {
      assert(FromEnum(folder) < static_cast<int32>(std::size(detail::init::g_specialFolderPaths)));

      return detail::init::g_specialFolderPaths[FromEnum(folder)];
    }

    bool Remove(const FilePathView path, const AllowUndo allowUndo)
    {
      if (not path)
      {
        return false;
      }

      if (IsResourcePath(path))
      {
        return false;
      }

      if (not allowUndo)
      {
        try
        {
          fs::remove_all(detail::ToPath(path));
          return true;
        }
        catch (const fs::filesystem_error&)
        {
          return false;
        }
      }

      return detail::Linux_TrashFile(Unicode::ToUTF8(path).c_str());
    }
  }
}
