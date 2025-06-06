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

# pragma once
# include <memory>
# include "Common.hpp"
# include "IReader.hpp"
# include "StringView.hpp"
# include "OpenMode.hpp"

namespace s3d
{
	class String;
	using FilePath = String;
	class Blob;

	////////////////////////////////////////////////////////////////
	//
	//	BinaryReader
	//
	////////////////////////////////////////////////////////////////

	/// @brief 読み込み用バイナリファイル
	class BinaryReader : public IReader
	{
	public:

		////////////////////////////////////////////////////////////////
		//
		//	(constructor)
		//
		////////////////////////////////////////////////////////////////

		/// @brief デフォルトコンストラクタ
		[[nodiscard]]
		BinaryReader();

		/// @brief ファイルを開きます。
		/// @param path ファイルパス
		[[nodiscard]]
		explicit BinaryReader(FilePathView path);

		////////////////////////////////////////////////////////////////
		//
		//	supportsLookahead
		//
		////////////////////////////////////////////////////////////////

		/// @brief lookahead をサポートしているかを返します。
		/// @return true
		[[nodiscard]]
		constexpr bool supportsLookahead() const noexcept override;

		////////////////////////////////////////////////////////////////
		//
		//	open
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルを開きます。
		/// @param path ファイルパス
		/// @return ファイルのオープンに成功した場合 true, それ以外の場合は false
		bool open(FilePathView path);

		////////////////////////////////////////////////////////////////
		//
		//	close
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルを閉じます。
		/// @remark ファイルが開いていない場合は何もしません。
		void close();

		////////////////////////////////////////////////////////////////
		//
		//	isOpen
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルが開いているかを返します。
		/// @return ファイルが開いている場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isOpen() const noexcept override;

		////////////////////////////////////////////////////////////////
		//
		//	operator bool
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルが開いているかを返します。
		/// @return ファイルが開いている場合 true, それ以外の場合は false	
		[[nodiscard]]
		explicit operator bool() const noexcept override;

		////////////////////////////////////////////////////////////////
		//
		//	size
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルのサイズを返します。
		/// @return ファイルのサイズ（バイト）
		[[nodiscard]]
		int64 size() const override;

		////////////////////////////////////////////////////////////////
		//
		//	getPos
		//
		////////////////////////////////////////////////////////////////

		/// @brief 現在の読み込み位置を返します。
		/// @return 現在の読み込み位置（バイト）		
		[[nodiscard]]
		int64 getPos() const override;

		////////////////////////////////////////////////////////////////
		//
		//	setPos
		//
		////////////////////////////////////////////////////////////////

		/// @brief 読み込み位置を変更します。
		/// @param pos 新しい読み込み位置（バイト）
		/// @return 新しい読み込み位置
		int64 setPos(int64 pos) override;

		////////////////////////////////////////////////////////////////
		//
		//	skip
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルを読み飛ばし、読み込み位置を変更します。
		/// @param offset 読み飛ばすサイズ（バイト）
		/// @return 新しい読み込み位置
		int64 skip(int64 offset) override;

		////////////////////////////////////////////////////////////////
		//
		//	read
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルからデータを読み込みます。
		/// @param dst 読み込み先
		/// @param size 読み込むサイズ（バイト）
		/// @return 実際に読み込んだサイズ（バイト）
		int64 read(void* dst, int64 size) override;

		/// @brief ファイルからデータを読み込みます。
		/// @param dst 読み込み先
		/// @param pos 先頭から数えた読み込み開始位置（バイト）
		/// @param size 読み込むサイズ（バイト）
		/// @return 実際に読み込んだサイズ（バイト）
		int64 read(void* dst, int64 pos, int64 size) override;

		/// @brief ファイルからデータを読み込みます。
		/// @param dst 読み込み先
		/// @return 読み込みに成功したら true, それ以外の場合は false
		bool read(Concept::TriviallyCopyable auto& dst);

		////////////////////////////////////////////////////////////////
		//
		//	readBlob
		//
		////////////////////////////////////////////////////////////////

		/// @brief ファイルの終端までのデータを読み込み、Blob として返します。
		/// @return 読み込んだデータ
		[[nodiscard]]
		Blob readBlob();

		/// @brief ファイルからデータを読み込み、Blob として返します。
		/// @param size 読み込むサイズ（バイト）
		/// @return 読み込んだデータ
		[[nodiscard]]
		Blob readBlob(int64 size);

		/// @brief ファイルからデータを読み込み、Blob として返します。
		/// @param pos 先頭から数えた読み込み開始位置（バイト）
		/// @param size 読み込むサイズ（バイト）
		/// @return 読み込んだデータ
		[[nodiscard]]
		Blob readBlob(int64 pos, int64 size);

		////////////////////////////////////////////////////////////////
		//
		//	lookahead
		//
		////////////////////////////////////////////////////////////////

		/// @brief 読み込み位置を動かさずにファイルからデータを読み込みます。
		/// @param dst 読み込み先
		/// @param size 読み込むサイズ（バイト）
		/// @return 実際に読み込んだサイズ（バイト）
		int64 lookahead(void* dst, int64 size) const override;

		/// @brief 読み込み位置を動かさずにファイルからデータを読み込みます。
		/// @param dst 読み込み先
		/// @param pos 先頭から数えた読み込み開始位置（バイト）
		/// @param size 読み込むサイズ（バイト）
		/// @return 実際に読み込んだサイズ（バイト）
		int64 lookahead(void* dst, int64 pos, int64 size) const override;

		/// @brief 読み込み位置を動かさずにファイルからデータを読み込みます。
		/// @tparam TriviallyCopyable 読み込む値の型
		/// @param dst 読み込み先
		/// @return 読み込みに成功したら true, それ以外の場合は false
		bool lookahead(Concept::TriviallyCopyable auto& dst) const;

		////////////////////////////////////////////////////////////////
		//
		//	lookaheadBlob
		//
		////////////////////////////////////////////////////////////////
		
		/// @brief 読み込み位置を動かさずにファイルからデータを読み込み、Blob として返します。
		/// @param size 読み込むサイズ（バイト）
		/// @return 読み込んだデータ
		[[nodiscard]]
		Blob lookaheadBlob(int64 size);
		
		/// @brief 読み込み位置を動かさずにファイルからデータを読み込み、Blob として返します。
		/// @param pos 先頭から数えた読み込み開始位置（バイト）
		/// @param size 読み込むサイズ（バイト）
		/// @return 読み込んだデータ
		[[nodiscard]]
		Blob lookaheadBlob(int64 pos, int64 size);

		////////////////////////////////////////////////////////////////
		//
		//	path
		//
		////////////////////////////////////////////////////////////////

		/// @brief 開いているファイルのパスを返します。
		/// @return 開いているファイルのパス。ファイルが開いていない場合は空の文字列
		[[nodiscard]]
		const FilePath& path() const noexcept;

	private:

		class BinaryReaderDetail;

		std::shared_ptr<BinaryReaderDetail> pImpl;
	};
}

# include "detail/BinaryReader.ipp"
