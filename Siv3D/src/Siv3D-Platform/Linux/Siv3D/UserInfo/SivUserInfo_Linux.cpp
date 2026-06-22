//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <climits>
# include <cstdlib>
# include <unistd.h>
# include <pwd.h>
# include <Siv3D/UserInfo.hpp>
# include <Siv3D/Unicode.hpp>

namespace s3d
{
	namespace
	{
		[[nodiscard]]
		static String ComputerName()
		{
			char hostname[HOST_NAME_MAX + 1] = {};

			if (::gethostname(hostname, sizeof(hostname)) != 0)
			{
				return{};
			}

			return Unicode::FromUTF8(hostname);
		}

		[[nodiscard]]
		static const ::passwd* GetPasswd() noexcept
		{
			return ::getpwuid(::geteuid());
		}

		[[nodiscard]]
		static String UserName()
		{
			if (const ::passwd* pw = GetPasswd(); pw && pw->pw_name)
			{
				return Unicode::FromUTF8(pw->pw_name);
			}

			if (const char* user = std::getenv("USER"))
			{
				return Unicode::FromUTF8(user);
			}

			return{};
		}

		[[nodiscard]]
		static String FullUserName()
		{
			if (const ::passwd* pw = GetPasswd(); pw && pw->pw_gecos && pw->pw_gecos[0])
			{
				// GECOS フィールドの最初のカンマまでがフルネーム。
				std::string gecos = pw->pw_gecos;

				if (const size_t comma = gecos.find(','); comma != std::string::npos)
				{
					gecos.resize(comma);
				}

				if (not gecos.empty())
				{
					return Unicode::FromUTF8(gecos);
				}
			}

			return UserName();
		}

		// "en_US.UTF-8" のようなロケール文字列から言語・地域コードを取り出す。
		[[nodiscard]]
		static String GetLocaleString()
		{
			const char* locale = std::getenv("LC_ALL");

			if ((not locale) || (locale[0] == '\0'))
			{
				locale = std::getenv("LANG");
			}

			if ((not locale) || (locale[0] == '\0') || (std::string_view{ locale } == "C") || (std::string_view{ locale } == "POSIX"))
			{
				return{};
			}

			std::string s = locale;

			// エンコーディング部（".UTF-8"）と修飾子（"@euro"）を除去。
			if (const size_t dot = s.find('.'); dot != std::string::npos)
			{
				s.resize(dot);
			}

			if (const size_t at = s.find('@'); at != std::string::npos)
			{
				s.resize(at);
			}

			return Unicode::FromUTF8(s);
		}

		[[nodiscard]]
		static String DefaultLocale()
		{
			const String locale = GetLocaleString();

			if (not locale)
			{
				return U"en-US";
			}

			// "en_US" -> "en-US"
			return locale.replaced(U'_', U'-');
		}

		[[nodiscard]]
		static String DefaultLanguage()
		{
			const String locale = DefaultLocale();

			if (not locale)
			{
				return U"en-US";
			}

			return locale;
		}
	}

	namespace System
	{
		////////////////////////////////////////////////////////////////
		//
		//	GetUserInfo
		//
		////////////////////////////////////////////////////////////////

		const UserInfo& GetUserInfo()
		{
			static const UserInfo userInfo = []()
			{
				UserInfo info;
				info.computerName		= ComputerName();
				info.userName			= UserName();
				info.fullUserName		= FullUserName();
				info.defaultLocale		= DefaultLocale();
				info.defaultLanguage	= DefaultLanguage();
				return info;
			}();

			return userInfo;
		}

		////////////////////////////////////////////////////////////////
		//
		//	IsRunningInVisualStudio
		//
		////////////////////////////////////////////////////////////////

		bool IsRunningInVisualStudio()
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		//
		//	IsRunningInXcode
		//
		////////////////////////////////////////////////////////////////

		bool IsRunningInXcode()
		{
			return false;
		}
	}
}
