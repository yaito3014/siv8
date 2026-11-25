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

# pragma once

# include <string_view>
# include <variant>

# include "String.hpp"
# include "JSON.hpp"
# include "Optional.hpp"

namespace s3d
{
	namespace detail
	{
		inline constexpr std::size_t NotArray = static_cast<std::size_t>(-1);
	}

	struct JSONItem
	{
		String key;

		JSON value;
	};

	class JSONIterator
	{
	public:
		struct JSONItemRef
		{
			String key;

			JSONRef value;

			[[nodiscard]]
			operator JSONItem() const;

			JSONItemRef& operator =(const JSONItemRef&) = delete;
		};

		using value_type = JSONItem;

		using difference_type = std::ptrdiff_t;

		[[nodiscard]]
		JSONIterator();

		[[nodiscard]]
		String key() const;

		[[nodiscard]]
		JSONRef value() const;

		[[nodiscard]]
		JSONItemRef operator *() const;

		JSONIterator& operator ++();

		JSONIterator operator ++(int);

		JSONIterator& operator --();

		JSONIterator operator --(int);

		friend bool operator ==(const JSONIterator& lhs, const JSONIterator& rhs) noexcept = default;

	private:

		friend JSON;

		friend JSONRef;

		friend JSONConstIterator;

		template <class>
		friend class detail::JSONMixin;

		JSONIterator(detail::JSONBase::iterator itr, std::size_t index);

		detail::JSONBase::iterator m_iterator;

		std::size_t m_index;
	};

	class JSONConstIterator
	{
	public:

		struct JSONItemConstRef
		{
			String key;

			JSONConstRef value;

			[[nodiscard]]
			operator JSONItem() const;
		};

		using value_type = JSONItem;

		using difference_type = std::ptrdiff_t;

		[[nodiscard]]
		JSONConstIterator();

		[[nodiscard]]
		JSONConstIterator(const JSONIterator&);

		[[nodiscard]]
		String key() const;

		[[nodiscard]]
		JSONConstRef value() const;

		[[nodiscard]]
		JSONItemConstRef operator *() const;

		JSONConstIterator& operator ++();

		JSONConstIterator operator ++(int);

		JSONConstIterator& operator --();

		JSONConstIterator operator --(int);

		friend bool operator ==(const JSONConstIterator& lhs, const JSONConstIterator& rhs) noexcept = default;

	private:

		friend JSON;

		template <class>
		friend class detail::JSONMixin;

		JSONConstIterator(detail::JSONBase::const_iterator itr, std::size_t index);

		detail::JSONBase::const_iterator m_iterator;

		std::size_t m_index;
	};
}

# include "detail/JSONIterator.ipp"
