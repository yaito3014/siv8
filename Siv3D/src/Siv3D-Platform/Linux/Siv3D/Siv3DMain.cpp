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

# include <iostream>
# include <Siv3D/System/ISystem.hpp>
# include <Siv3D/Error.hpp>
# include <Siv3D/Engine/Siv3DEngine.hpp>
# include <Siv3D/FreestandingMessageBox/FreestandingMessageBox.hpp>
# include <Siv3D/System/ExitCode.hpp>
# include <Siv3D/EngineLog.hpp>
# include <Siv3D/FileSystem.hpp>

void Main();

namespace s3d::detail::init
{
	void InitCommandLines(int argc, char** argv);
}

int main(int argc, char* argv[])
{
	using namespace s3d;

	std::clog << "Siv3D for Linux\n";

	detail::init::InitCommandLines(argc, argv);

	// Resolve relative resource paths against the executable's directory rather
	// than the shell's current directory (mirrors WindowsDesktop's
	// SetWorkingDirectory() and the macOS startup chdir). The engine loads its
	// own resources (shaders, fonts) via relative paths during System::init(),
	// so this must happen before it.
	if (const FilePath workingDirectory = FileSystem::GetExecutableDirectory())
	{
		FileSystem::ChangeCurrentDirectory(workingDirectory);
	}

	Siv3DEngine engine;

	try
	{
		SIV3D_ENGINE(System)->init();
	}
	catch (const Error& error)
	{
		FreestandingMessageBox::ShowError(error.messageUTF8());
		std::cerr << error << '\n';
		return -1;
	}

	LOG_DEBUG("Main() ---");

	Main();

	LOG_DEBUG("--- Main()");

	return GetExitCode();
}
