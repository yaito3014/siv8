//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2025 Ryo Suzuki
//	Copyright (c) 2016-2025 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "ChildProcessDetail.hpp"

namespace s3d
{
	ChildProcess::ChildProcessDetail::ChildProcessDetail()
	{

	}

	ChildProcess::ChildProcessDetail::ChildProcessDetail(const FilePathView path, const Array<String>& commands, const Pipe pipe)
	{

	}

	ChildProcess::ChildProcessDetail::~ChildProcessDetail()
	{

	}

	bool ChildProcess::ChildProcessDetail::isValid() const
	{
		return false;
	}

	bool ChildProcess::ChildProcessDetail::isRunning()
	{
		return false;
	}

	void ChildProcess::ChildProcessDetail::wait()
	{
		
	}

	void ChildProcess::ChildProcessDetail::terminate()
	{
		
	}

	Optional<int32> ChildProcess::ChildProcessDetail::getExitCode()
	{
		return EXIT_FAILURE;
	}

	std::istream& ChildProcess::ChildProcessDetail::istream()
	{
		return std::cin;
	}

	std::ostream& ChildProcess::ChildProcessDetail::ostream()
	{
		return std::cout;
	}
}
