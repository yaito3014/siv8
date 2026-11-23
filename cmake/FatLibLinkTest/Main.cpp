# include <Siv3D.hpp>
# include <string>
# include <vector>

void Main()
{
}

// Definitions the fat lib references but does not contain, each a known gap
// recorded in TODO.md. Nothing calls them; they only let the link complete.
// Once a gap is fixed, its stub collides with the real definition and the
// link fails, so remove the stub then.

namespace s3d
{
# if not SIV3D_PLATFORM(MACOS)

	// The overloads taking a title are implemented for macOS only.
	MessageBoxResult System::MessageBoxOK(StringView, StringView, MessageBoxStyle)
	{
		return MessageBoxResult::Ok;
	}

	MessageBoxResult System::MessageBoxOKCancel(StringView, StringView, MessageBoxStyle)
	{
		return MessageBoxResult::Cancel;
	}

	MessageBoxResult System::MessageBoxYesNo(StringView, StringView, MessageBoxStyle)
	{
		return MessageBoxResult::No;
	}

# endif

# if SIV3D_PLATFORM(LINUX)

	// Not implemented on Linux yet.
	Optional<FilePath> Dialog::OpenFile(const OpenFileDialogOptions&)
	{
		return none;
	}

	Optional<FilePath> Dialog::SaveFile(const SaveFileDialogOptions&)
	{
		return none;
	}

	Array<MonitorInfo> System::EnumerateMonitors()
	{
		return{};
	}

	size_t System::GetCurrentMonitorIndex()
	{
		return 0;
	}

# endif
}

// The bc7enc-rdo overlay port leaves out lodepng.cpp and bc7decomp_ref.cpp,
// which its utils and encoder code call. Declared to match their headers,
// which the port does not install.

enum LodePNGColorType : int;

namespace lodepng
{
	unsigned decode(std::vector<unsigned char>&, unsigned&, unsigned&, const std::string&, LodePNGColorType, unsigned)
	{
		return 1;
	}

	unsigned encode(const std::string&, const std::vector<unsigned char>&, unsigned, unsigned, LodePNGColorType, unsigned)
	{
		return 1;
	}
}

namespace bc7decomp
{
	class color_rgba;
}

namespace bc7decomp_ref
{
	bool unpack_bc7(const void*, bc7decomp::color_rgba*)
	{
		return false;
	}
}
