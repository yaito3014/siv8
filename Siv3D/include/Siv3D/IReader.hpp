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
# include "Concepts.hpp"

namespace s3d
{
	////////////////////////////////////////////////////////////////
	//
	//	IReader
	//
	////////////////////////////////////////////////////////////////

	/// @brief Reader インタフェース | Reader interface
	class IReader
	{
	public:

		////////////////////////////////////////////////////////////////
		//
		//	(destructor)
		//
		////////////////////////////////////////////////////////////////

		/// @brief デストラクタ  | Destructor
		virtual ~IReader() = default;

		////////////////////////////////////////////////////////////////
		//
		//	supportsLookahead
		//
		////////////////////////////////////////////////////////////////

		/// @brief Reader が読み込み位置を前進させないデータ読み込みをサポートしているかを返します。 | Returns whether the Reader supports data reading without advancing the read position.
		/// @return 読み込み位置を前進させないデータ読み込みをサポートしている場合 true, それ以外の場合は false | Returns true if the Reader supports data reading without advancing the read position, otherwise false
		[[nodiscard]]
		virtual bool supportsLookahead() const noexcept = 0;

		////////////////////////////////////////////////////////////////
		//
		//	isOpen
		//
		////////////////////////////////////////////////////////////////

		/// @brief Reader のデータにアクセス可能かを返します。 | Returns whether the Reader can access the data.
		/// @return データにアクセス可能な場合 true, それ以外の場合は false | Returns true if the data can be accessed, otherwise false
		[[nodiscard]]
		virtual bool isOpen() const noexcept = 0;

		////////////////////////////////////////////////////////////////
		//
		//	operator bool
		//
		////////////////////////////////////////////////////////////////

		/// @brief Reader のデータにアクセス可能かを返します。 | Returns whether the Reader can access the data.
		/// @return データにアクセス可能な場合 true, それ以外の場合は false | Returns true if the data can be accessed, otherwise false
		[[nodiscard]]
		virtual explicit operator bool() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	size
		//
		////////////////////////////////////////////////////////////////

		/// @brief データのサイズを返します。 | Returns the size of the data.
		/// @return データのサイズ（バイト） | The size of the data (bytes)
		[[nodiscard]]
		virtual int64 size() const = 0;

		////////////////////////////////////////////////////////////////
		//
		//	getPos
		//
		////////////////////////////////////////////////////////////////

		/// @brief データの現在の読み込み位置を返します。 | Returns the current read position of the data.
		/// @return 現在の読み込み位置（バイト） | The current read position (bytes)
		[[nodiscard]]
		virtual int64 getPos() const = 0;

		////////////////////////////////////////////////////////////////
		//
		//	setPos
		//
		////////////////////////////////////////////////////////////////

		/// @brief データの読み込み位置を変更します。 | Changes the read position of the data.
		/// @param pos 新しい読み込み位置（バイト） | The new read position (bytes)
		/// @return 新しい読み込み位置（バイト） | The new read position (bytes)
		virtual int64 setPos(int64 pos) = 0;

		////////////////////////////////////////////////////////////////
		//
		//	skip
		//
		////////////////////////////////////////////////////////////////

		/// @brief データを読み飛ばし、読み込み位置を前進させます。 | Skips the data and advances the read position.
		/// @param offset 読み飛ばすサイズ（バイト） | The size to skip (bytes)
		/// @return 新しい読み込み位置（バイト） | The new read position (bytes)
		virtual int64 skip(int64 offset) = 0;

		////////////////////////////////////////////////////////////////
		//
		//	read
		//
		////////////////////////////////////////////////////////////////

		/// @brief データを読み込み、その分読み込み位置を前進させます。 | Reads the data and advances the read position.
		/// @param dst 読み込んだデータの格納先 | The destination to store the read data
		/// @param size 読み込むサイズ（バイト） | The size to read (bytes)
		/// @return 実際に読み込んだサイズ（バイト） | The actual size read (bytes)
		virtual int64 read(void* dst, int64 size) = 0;

		/// @brief データを読み込み、その分読み込み位置を前進させます。 | Reads the data and advances the read position.
		/// @param dst 読み込んだデータの格納先 | The destination to store the read data
		/// @param pos 先頭から数えた読み込み開始位置（バイト） | The read start position from the beginning (bytes)
		/// @param size 読み込むサイズ（バイト） | The size to read (bytes)
		/// @return 実際に読み込んだサイズ（バイト） | The actual size read (bytes)
		virtual int64 read(void* dst, int64 pos, int64 size) = 0;

		/// @brief データを読み込み、その分読み込み位置を前進させます。 | Reads the data and advances the read position.
		/// @param dst 読み込んだデータの格納先 | The destination to store the read data
		/// @return 読み込みに成功した場合 true, それ以外の場合は false | Returns true if the read was successful, otherwise false
		bool read(Concept::TriviallyCopyable auto& dst);

		////////////////////////////////////////////////////////////////
		//
		//	lookahead
		//
		////////////////////////////////////////////////////////////////

		/// @brief 現在の読み込み位置から、読み込み位置を前進させずにデータを読み込みます。 | Reads the data from the current read position without advancing the read position.
		/// @param dst 読み込んだデータの格納先 | The destination to store the read data
		/// @param size 読み込むサイズ（バイト） | The size to read (bytes)
		/// @return 実際に読み込んだサイズ（バイト） | The actual size read (bytes)
		virtual int64 lookahead(void* dst, int64 size) const = 0;

		/// @brief 現在の読み込み位置は変更せずに、データを読み込みます。 | Reads the data without changing the current read position.
		/// @param dst 読み込んだデータの格納先 | The destination to store the read data
		/// @param pos 先頭から数えた読み込み開始位置（バイト） | The read start position from the beginning (bytes)
		/// @param size 読み込むサイズ（バイト） | The size to read (bytes)
		/// @return 実際に読み込んだサイズ（バイト） | The actual size read (bytes)
		virtual int64 lookahead(void* dst, int64 pos, int64 size) const = 0;

		/// @brief 現在の読み込み位置から、読み込み位置を前進させずにデータを読み込みます。 | Reads the data from the current read position without advancing the read position.
		/// @param dst 読み込んだデータの格納先 | The destination to store the read data
		/// @return 読み込みに成功したら true, それ以外の場合は false | Returns true if the read was successful, otherwise false
		bool lookahead(Concept::TriviallyCopyable auto& dst) const;
	};
}

# include "detail/IReader.ipp"
