//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2016-2026 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <Siv3D/DragDrop/IDragDrop.hpp>

namespace s3d
{
	// TODO(linux): no drag & drop backend yet (GLFW only delivers file drops).
	class CDragDrop final : public ISiv3DDragDrop
	{
	public:

		void init() override {}

		void update() override {}

		bool isAcceptingFilePaths() const override { return false; }

		bool isAcceptingText() const override { return false; }

		void acceptFilePaths(bool) override {}

		void acceptText(bool) override {}

		Optional<DragStatus> dragOver() const override { return none; }

		bool hasNewFilePaths() const override { return false; }

		bool hasNewText() const override { return false; }

		void clear() override {}

		Array<DroppedFilePath> extractDroppedFilePaths() override { return{}; }

		Array<DroppedText> extractDroppedTexts() override { return{}; }

		bool beginDragFile(FilePathView) override { return false; }

		bool beginDragFiles(const Array<FilePath>&) override { return false; }

		bool beginDragText(StringView) override { return false; }
	};
}
