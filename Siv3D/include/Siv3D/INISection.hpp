﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2024 Ryo Suzuki
//	Copyright (c) 2016-2024 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include "INIItem.hpp"
# include "HashTable.hpp"

namespace s3d
{
	////////////////////////////////////////////////////////////////
	//
	//	INISection
	//
	////////////////////////////////////////////////////////////////

	/// @brief INI ファイルのセクション
	struct INISection
	{
		using iterator = HashTable<String, INIItem>::iterator;

		using const_iterator = HashTable<String, INIItem>::const_iterator;

		/// @brief セクション名
		String name;

		/// @brief プロパティ
		HashTable<String, INIItem> items;

		////////////////////////////////////////////////////////////////
		//
		//	hasProperty
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		bool hasProperty(StringView key) const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	addProperty
		//
		////////////////////////////////////////////////////////////////

		void addProperty(StringView key, const Concept::Formattable auto& value);

		////////////////////////////////////////////////////////////////
		//
		//	removeProperty
		//
		////////////////////////////////////////////////////////////////

		void removeProperty(StringView key);

		////////////////////////////////////////////////////////////////
		//
		//	operator []
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		const INIItem& operator [](StringView key) const;

		[[nodiscard]]
		INIItem& operator [](StringView key);

		////////////////////////////////////////////////////////////////
		//
		//	begin, end
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		iterator begin() noexcept;

		[[nodiscard]]
		iterator end() noexcept;

		[[nodiscard]]
		const_iterator begin() const noexcept;

		[[nodiscard]]
		const_iterator end() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	cbegin, cend
		//
		////////////////////////////////////////////////////////////////

		[[nodiscard]]
		const_iterator cbegin() const noexcept;

		[[nodiscard]]
		const_iterator cend() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	get
		//
		////////////////////////////////////////////////////////////////

		template <class Type>
		[[nodiscard]]
		Type get(StringView key) const;

		////////////////////////////////////////////////////////////////
		//
		//	getOr
		//
		////////////////////////////////////////////////////////////////

		template <class Type, class U>
		[[nodiscard]]
		Type getOr(StringView key, U&& defaultValue) const;

		////////////////////////////////////////////////////////////////
		//
		//	getOpt
		//
		////////////////////////////////////////////////////////////////

		template <class Type>
		[[nodiscard]]
		Optional<Type> getOpt(StringView key) const;

		////////////////////////////////////////////////////////////////
		//
		//	format
		//
		////////////////////////////////////////////////////////////////

		String format() const;

	private:

		void _addProperty(StringView key, String&& value);
	};
}

# include "detail/INISection.ipp"