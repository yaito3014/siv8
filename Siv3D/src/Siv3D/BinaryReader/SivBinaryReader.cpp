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

# include <Siv3D/BinaryReader.hpp>
# include <Siv3D/Error.hpp>
# include <Siv3D/Blob.hpp>
# include <Siv3D/FormatLiteral.hpp>
# include <Siv3D/BinaryReader/BinaryReaderDetail.hpp>

namespace s3d
{
	namespace
	{
		[[noreturn]]
		static void ThrowReadDstError()
		{
			throw Error{ "BinaryReader::read(): A non-null destination pointer is required when readSize is greater than 0." };
		}

		[[noreturn]]
		static void ThrowReadRangeError(const int64 pos, const int64 fileSize)
		{
			throw Error{ fmt::format("BinaryReader::read(): Position ({0}) is out of the valid range. The file size is {1} bytes.", pos, fileSize) };
		}

		[[noreturn]]
		static void ThrowLookaheadDstError()
		{
			throw Error{ "BinaryReader::lookahead(): A non-null destination pointer is required when readSize is greater than 0." };
		}

		[[noreturn]]
		static void ThrowLookaheadRangeError(const int64 pos, const int64 fileSize)
		{
			throw Error{ fmt::format("BinaryReader::lookahead(): Position ({0}) is out of the valid range. The file size is {1} bytes.", pos, fileSize) };
		}
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	(constructor)
	//
	////////////////////////////////////////////////////////////////

	BinaryReader::BinaryReader()
		: pImpl{ std::make_shared<BinaryReaderDetail>() } {}

	BinaryReader::BinaryReader(const FilePathView path)
		: BinaryReader{}
	{
		open(path);
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	open
	//
	////////////////////////////////////////////////////////////////

	bool BinaryReader::open(const FilePathView path)
	{
		return pImpl->open(path);
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	close
	//
	////////////////////////////////////////////////////////////////

	void BinaryReader::close()
	{
		pImpl->close();
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	isOpen
	//
	////////////////////////////////////////////////////////////////

	bool BinaryReader::isOpen() const noexcept
	{
		return pImpl->isOpen();
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	operator bool
	//
	////////////////////////////////////////////////////////////////

	BinaryReader::operator bool() const noexcept
	{
		return pImpl->isOpen();
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	size
	//
	////////////////////////////////////////////////////////////////

	int64 BinaryReader::size() const
	{
		return pImpl->size();
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	getPos
	//
	////////////////////////////////////////////////////////////////

	int64 BinaryReader::getPos() const
	{
		return pImpl->getPos();
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	setPos
	//
	////////////////////////////////////////////////////////////////

	int64 BinaryReader::setPos(const int64 pos)
	{
		return pImpl->setPos(pos);
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	skip
	//
	////////////////////////////////////////////////////////////////

	int64 BinaryReader::skip(const int64 offset)
	{
		return pImpl->skip(offset);
	}
		
	////////////////////////////////////////////////////////////////
	//
	//	read
	//
	////////////////////////////////////////////////////////////////

	int64 BinaryReader::read(void* const dst, const int64 readSize)
	{
		if (readSize <= 0)
		{
			return 0;
		}

		if (dst == nullptr)
		{
			ThrowReadDstError();
		}

		return pImpl->read(NonNull{ dst }, readSize);
	}

	int64 BinaryReader::read(void* const dst, const int64 pos, const int64 readSize)
	{
		if (readSize <= 0)
		{
			return 0;
		}

		if (dst == nullptr)
		{
			ThrowReadDstError();
		}

		if (const int64 fileSize = pImpl->size();
			not InRange<int64>(pos, 0, fileSize))
		{
			ThrowReadRangeError(pos, fileSize);
		}

		return pImpl->read(NonNull{ dst }, pos, readSize);
	}

	////////////////////////////////////////////////////////////////
	//
	//	readBlob
	//
	////////////////////////////////////////////////////////////////

	Blob BinaryReader::readBlob()
	{
		const int64 toReadBytes = (size() - getPos());
		Blob blob(toReadBytes);

		const int64 readBytes = read(blob.data(), toReadBytes);
		blob.resize(readBytes);

		return blob;
	}

	Blob BinaryReader::readBlob(const int64 _size)
	{
		const int64 toReadBytes = Min(_size, (size() - getPos()));
		Blob blob(toReadBytes);

		const int64 readBytes = read(blob.data(), toReadBytes);
		blob.resize(readBytes);

		return blob;
	}

	Blob BinaryReader::readBlob(const int64 pos, const int64 _size)
	{
		const int64 fileSize = pImpl->size();

		if (not InRange<int64>(pos, 0, fileSize))
		{
			ThrowReadRangeError(pos, fileSize);
		}

		const int64 toReadBytes = Min(_size, (fileSize - pos));
		Blob blob(toReadBytes);
		
		const int64 readBytes = read(blob.data(), pos, toReadBytes);
		blob.resize(readBytes);
		
		return blob;
	}

	////////////////////////////////////////////////////////////////
	//
	//	lookahead
	//
	////////////////////////////////////////////////////////////////

	int64 BinaryReader::lookahead(void* const dst, const int64 readSize) const
	{
		if (readSize <= 0)
		{
			return 0;
		}

		if (dst == nullptr)
		{
			ThrowLookaheadDstError();
		}

		return pImpl->lookahead(NonNull{ dst }, readSize);
	}

	int64 BinaryReader::lookahead(void* const dst, const int64 pos, const int64 readSize) const
	{
		if (readSize <= 0)
		{
			return 0;
		}

		if (dst == nullptr)
		{
			ThrowLookaheadDstError();
		}

		if (const int64 fileSize = pImpl->size();
			not InRange<int64>(pos, 0, fileSize))
		{
			ThrowLookaheadRangeError(pos, fileSize);
		}

		return pImpl->lookahead(NonNull{ dst }, pos, readSize);
	}

	////////////////////////////////////////////////////////////////
	//
	//	lookaheadBlob
	//
	////////////////////////////////////////////////////////////////

	Blob BinaryReader::lookaheadBlob(const int64 _size)
	{
		const int64 toReadBytes = Min(_size, (size() - getPos()));
		Blob blob(toReadBytes);

		const int64 readBytes = lookahead(blob.data(), toReadBytes);
		blob.resize(readBytes);

		return blob;
	}

	Blob BinaryReader::lookaheadBlob(const int64 pos, const int64 _size)
	{
		const int64 fileSize = pImpl->size();
		
		if (not InRange<int64>(pos, 0, fileSize))
		{
			ThrowLookaheadRangeError(pos, fileSize);
		}
		
		const int64 toReadBytes = Min(_size, (fileSize - pos));
		Blob blob(toReadBytes);
		
		const int64 readBytes = lookahead(blob.data(), pos, toReadBytes);
		blob.resize(readBytes);
		
		return blob;
	}

	////////////////////////////////////////////////////////////////
	//
	//	path
	//
	////////////////////////////////////////////////////////////////

	const FilePath& BinaryReader::path() const noexcept
	{
		return pImpl->path();
	}
}
