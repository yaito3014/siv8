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

# include <Siv3D/MemoryMappedFileView.hpp>
# include <Siv3D/Windows/Windows.hpp>

namespace s3d
{
	class MemoryMappedFileView::MemoryMappedFileViewDetail
	{
	public:

		[[nodiscard]]
		MemoryMappedFileViewDetail() = default;

		~MemoryMappedFileViewDetail();

		bool open(FilePathView path);

		void close();

		[[nodiscard]]
		bool isOpen() const;

		[[nodiscard]]
		MappedMemoryView map(size_t offset, size_t requestSize);

		void unmap();

		[[nodiscard]]
		int64 size() const;

		[[nodiscard]]
		const FilePath& path() const;

	private:

		struct File
		{
			HANDLE fileHandle = INVALID_HANDLE_VALUE;

			HANDLE fileMapping = nullptr;

			const Byte* baseAddress	= nullptr;

		} m_file;

		struct Resource
		{
			const Byte* pointer = nullptr;

		} m_resource;

		struct Info
		{
			FilePath fullPath;

			int64 fileSize = 0;

			bool isOpen = false;

		} m_info;

		[[nodiscard]]
		bool isResource() const noexcept;
	};
}
