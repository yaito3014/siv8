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

namespace s3d
{
	namespace detail
	{
		template <class>
		inline constexpr bool TOMLAlwaysFalse = false;

		template <class Derived>
		inline const TOMLValue& TOMLMixin<Derived>::getValue() const noexcept
		{
			return static_cast<const Derived&>(*this).getValueImpl();
		}

		template <class Derived>
		inline const TOMLValue* TOMLMixin<Derived>::getRoot() const noexcept
		{
			return static_cast<const Derived&>(*this).getRootImpl();
		}

		template <class Derived>
		template <class Type>
		Optional<Type> TOMLMixin<Derived>::getOpt() const
		{
			const TOMLValue& value = getValue();

			if constexpr (std::is_same_v<Type, TOML>)
			{
				if (value.is_empty())
				{
					return none;
				}

				return TOML::FromValue(value);
			}
			else if constexpr (std::is_same_v<Type, String>)
			{
				if (not value.is_string())
				{
					return none;
				}

				return Unicode::FromUTF8(value.as_string());
			}
			else if constexpr (std::is_same_v<Type, std::string>)
			{
				if (not value.is_string())
				{
					return none;
				}

				return value.as_string();
			}
			else if constexpr (std::is_same_v<Type, bool>)
			{
				if (not value.is_boolean())
				{
					return none;
				}

				return value.as_boolean();
			}
			else if constexpr (std::is_integral_v<Type>)
			{
				if (not value.is_integer())
				{
					return none;
				}

				const int64 i = value.as_integer();

				if constexpr (std::is_signed_v<Type>)
				{
					if ((i < static_cast<int64>((std::numeric_limits<Type>::min)()))
						|| (static_cast<int64>((std::numeric_limits<Type>::max)()) < i))
					{
						return none;
					}
				}
				else
				{
					if (i < 0)
					{
						return none;
					}

					if (static_cast<uint64>((std::numeric_limits<Type>::max)()) < static_cast<uint64>(i))
					{
						return none;
					}
				}

				return static_cast<Type>(i);
			}
			else if constexpr (std::is_floating_point_v<Type>)
			{
				if (value.is_floating())
				{
					return static_cast<Type>(value.as_floating());
				}

				if (value.is_integer())
				{
					return static_cast<Type>(value.as_integer());
				}

				return none;
			}
			else if constexpr (std::is_same_v<Type, TOMLLocalTime>)
			{
				if (not value.is_local_time())
				{
					return none;
				}

				return TOMLLocalTime::FromTOML(value.as_local_time());
			}
			else if constexpr (std::is_same_v<Type, Date>)
			{
				if (not value.is_local_date())
				{
					return none;
				}

				return ToDate(value.as_local_date());
			}
			else if constexpr (std::is_same_v<Type, TOMLLocalDateTime>)
			{
				if (not value.is_local_datetime())
				{
					return none;
				}

				return TOMLLocalDateTime::FromTOML(value.as_local_datetime());
			}
			else if constexpr (std::is_same_v<Type, TOMLOffsetDateTime>)
			{
				if (not value.is_offset_datetime())
				{
					return none;
				}

				return TOMLOffsetDateTime::FromTOML(value.as_offset_datetime());
			}
			else
			{
				static_assert(TOMLAlwaysFalse<Type>, "Unsupported TOML::getOpt<Type>(). Use getParsedOpt<Type>() for string-based parsing.");
			}
		}

		template <class Derived>
		template <class Type>
		Type TOMLMixin<Derived>::get() const
		{
			if (auto opt = getOpt<Type>())
			{
				return *std::move(opt);
			}

			ThrowTOMLGetError(typeid(Type).name(), getRoot(), getValue());
		}

		template <class Derived>
		template <class Type, class U>
		Type TOMLMixin<Derived>::getOr(U&& defaultValue) const
		{
			if (auto opt = getOpt<Type>())
			{
				return *std::move(opt);
			}

			return Type{ std::forward<U>(defaultValue) };
		}

		template <class Derived>
		template <class Type>
		Optional<Type> TOMLMixin<Derived>::getParsedOpt() const
		{
			if (not isString())
			{
				return none;
			}

			return ParseOpt<Type>(getString());
		}

		template <class Derived>
		template <class Type>
		Type TOMLMixin<Derived>::getParsed() const
		{
			if (auto opt = getParsedOpt<Type>())
			{
				return *std::move(opt);
			}

			ThrowTOMLParsedGetError(typeid(Type).name(), getRoot(), getValue());
		}

		template <class Derived>
		template <class Type, class U>
		Type TOMLMixin<Derived>::getParsedOr(U&& defaultValue) const
		{
			if (auto opt = getParsedOpt<Type>())
			{
				return *std::move(opt);
			}

			return Type{ std::forward<U>(defaultValue) };
		}

		template <class Derived>
		template <class Type>
		Optional<Array<Type>> TOMLMixin<Derived>::getArrayOpt() const
		{
			const TOMLValue& value = getValue();

			if (not value.is_array())
			{
				return none;
			}

			const auto& array = value.as_array();
			Array<Type> result;
			result.reserve(array.size());

			for (const auto& element : array)
			{
				if (auto opt = TOMLConstRef{ getRoot(), &element }.template getOpt<Type>())
				{
					result.push_back(*std::move(opt));
				}
				else
				{
					return none;
				}
			}

			return result;
		}

		template <class Derived>
		template <class Type>
		Array<Type> TOMLMixin<Derived>::getArray() const
		{
			const TOMLValue& value = getValue();

			if (not value.is_array())
			{
				ThrowTOMLNotArray(typeid(Type).name(), getRoot(), value);
			}

			const auto& array = value.as_array();
			Array<Type> result;
			result.reserve(array.size());

			for (const auto& element : array)
			{
				if (auto opt = TOMLConstRef{ getRoot(), &element }.template getOpt<Type>())
				{
					result.push_back(*std::move(opt));
				}
				else
				{
					ThrowTOMLArrayElementGetError(typeid(Type).name(), getRoot(), element);
				}
			}

			return result;
		}

		template <class Derived>
		inline TOMLValue& TOMLMutableMixin<Derived>::getMutableValue() noexcept
		{
			return static_cast<Derived&>(*this).getMutableValueImpl();
		}

		template <class Derived>
		inline TOMLValue* TOMLMutableMixin<Derived>::getMutableRoot() noexcept
		{
			return static_cast<Derived&>(*this).getMutableRootImpl();
		}

		template <class Int>
		[[nodiscard]]
		std::int64_t ToTOMLInteger(const Int value)
		{
			if constexpr (std::is_unsigned_v<Int> && (sizeof(int64) <= sizeof(Int)))
			{
				if (static_cast<uint64>((std::numeric_limits<int64>::max)()) < static_cast<uint64>(value))
				{
					ThrowTOMLIntegerOutOfRange(static_cast<uint64>(value));
				}
			}

			return static_cast<std::int64_t>(value);
		}

		extern template class TOMLMixin<TOML>;
		extern template class TOMLMixin<TOMLRef>;
		extern template class TOMLMixin<TOMLConstRef>;
		extern template class TOMLMutableMixin<TOML>;
		extern template class TOMLMutableMixin<TOMLRef>;
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOML
	//
	////////////////////////////////////////////////////////////////

	template <std::same_as<bool> Bool>
	TOML::TOML(const Bool value)
		: m_value(static_cast<bool>(value)) {}

	template <class Int>
		requires (std::integral<Int> && (not std::same_as<Int, bool>) && (not std::same_as<Int, char>)
			&& (not std::same_as<Int, char8_t>) && (not std::same_as<Int, char16_t>)
			&& (not std::same_as<Int, char32_t>) && (not std::same_as<Int, wchar_t>))
	TOML::TOML(const Int value)
		: m_value(detail::ToTOMLInteger(value)) {}

	template <std::floating_point Float>
	TOML::TOML(const Float value)
		: m_value(static_cast<double>(value)) {}

	template <class Type>
		requires std::constructible_from<TOML, const Type&>
	TOML::TOML(const Array<Type>& array)
		: m_value(Value::array_type{})
	{
		auto& elements = m_value.as_array();
		elements.reserve(array.size());

		for (const auto& element : array)
		{
			if constexpr (std::is_same_v<Type, TOML>)
			{
				elements.push_back(element.m_value);
			}
			else
			{
				elements.push_back(TOML(element).m_value);
			}
		}
	}

	template <class Type, class Hash, class Eq, class Alloc>
		requires std::constructible_from<TOML, const Type&>
	TOML::TOML(const HashMap<String, Type, Hash, Eq, Alloc>& table)
		: m_value(Value::table_type{})
	{
		auto& members = m_value.as_table();

		for (const auto& [key, value] : table)
		{
			if constexpr (std::is_same_v<Type, TOML>)
			{
				members.emplace(Unicode::ToUTF8(key), value.m_value);
			}
			else
			{
				members.emplace(Unicode::ToUTF8(key), TOML(value).m_value);
			}
		}
	}

	template <ReaderObject Reader>
	TOML TOML::Load(Reader&& reader)
	{
		return Load(std::make_unique<Reader>(std::move(reader)));
	}

	template <ReaderObject Reader>
	Result<TOML, Array<TOMLParseErrorReason>> TOML::LoadResult(Reader&& reader)
	{
		return LoadResult(std::make_unique<Reader>(std::move(reader)));
	}

	inline const TOML::Value& TOML::getValueImpl() const noexcept
	{
		return m_value;
	}

	inline const TOML::Value* TOML::getRootImpl() const noexcept
	{
		return &m_value;
	}

	inline TOML::Value& TOML::getMutableValueImpl() noexcept
	{
		return m_value;
	}

	inline TOML::Value* TOML::getMutableRootImpl() noexcept
	{
		return &m_value;
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOMLRef
	//
	////////////////////////////////////////////////////////////////

	inline TOMLRef::TOMLRef(detail::TOMLValue* root, detail::TOMLValue* value) noexcept
		: m_root{ root }
		, m_value{ value } {}

	inline const detail::TOMLValue& TOMLRef::getValueImpl() const noexcept
	{
		return *m_value;
	}

	inline const detail::TOMLValue* TOMLRef::getRootImpl() const noexcept
	{
		return m_root;
	}

	inline detail::TOMLValue& TOMLRef::getMutableValueImpl() noexcept
	{
		return *m_value;
	}

	inline detail::TOMLValue* TOMLRef::getMutableRootImpl() noexcept
	{
		return m_root;
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOMLConstRef
	//
	////////////////////////////////////////////////////////////////

	inline TOMLConstRef::TOMLConstRef(const TOML& value) noexcept
		: m_root{ value.getRootImpl() }
		, m_value{ &value.getValueImpl() } {}

	inline TOMLConstRef::TOMLConstRef(const TOMLRef& value) noexcept
		: m_root{ value.m_root }
		, m_value{ value.m_value } {}

	inline TOMLConstRef::TOMLConstRef(const detail::TOMLValue* root, const detail::TOMLValue* value) noexcept
		: m_root{ root }
		, m_value{ value } {}

	inline const detail::TOMLValue& TOMLConstRef::getValueImpl() const noexcept
	{
		return *m_value;
	}

	inline const detail::TOMLValue* TOMLConstRef::getRootImpl() const noexcept
	{
		return m_root;
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOMLArrayView
	//
	////////////////////////////////////////////////////////////////

	template <bool Const>
	TOMLArrayView<Const>::Iterator::Iterator(const RootPointer root, const ArrayIterator it) noexcept
		: m_root{ root }
		, m_it{ it } {}

	template <bool Const>
	typename TOMLArrayView<Const>::Iterator& TOMLArrayView<Const>::Iterator::operator ++()
	{
		++m_it;
		return *this;
	}

	template <bool Const>
	typename TOMLArrayView<Const>::Iterator TOMLArrayView<Const>::Iterator::operator ++(int)
	{
		Iterator old = *this;
		++m_it;
		return old;
	}

	template <bool Const>
	typename TOMLArrayView<Const>::reference TOMLArrayView<Const>::Iterator::operator *() const
	{
		return reference{ m_root, &(*m_it) };
	}

	template <bool Const>
	TOMLArrayView<Const>::TOMLArrayView(const RootPointer root, ArrayType* array) noexcept
		: m_root{ root }
		, m_array{ array } {}

	template <bool Const>
	typename TOMLArrayView<Const>::Iterator TOMLArrayView<Const>::begin() const
	{
		return (m_array ? Iterator{ m_root, m_array->begin() } : Iterator{});
	}

	template <bool Const>
	typename TOMLArrayView<Const>::Iterator TOMLArrayView<Const>::end() const
	{
		return (m_array ? Iterator{ m_root, m_array->end() } : Iterator{});
	}

	template <bool Const>
	size_t TOMLArrayView<Const>::size() const noexcept
	{
		return (m_array ? m_array->size() : 0);
	}

	template <bool Const>
	bool TOMLArrayView<Const>::isEmpty() const noexcept
	{
		return (size() == 0);
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOMLTableView
	//
	////////////////////////////////////////////////////////////////

	template <bool Const>
	TOMLTableView<Const>::Iterator::Iterator(const RootPointer root, const TableIterator it) noexcept
		: m_root{ root }
		, m_it{ it } {}

	template <bool Const>
	typename TOMLTableView<Const>::Iterator& TOMLTableView<Const>::Iterator::operator ++()
	{
		++m_it;
		return *this;
	}

	template <bool Const>
	typename TOMLTableView<Const>::Iterator TOMLTableView<Const>::Iterator::operator ++(int)
	{
		Iterator old = *this;
		++m_it;
		return old;
	}

	template <bool Const>
	TOMLTableMember<Const> TOMLTableView<Const>::Iterator::operator *() const
	{
		using Reference = std::conditional_t<Const, TOMLConstRef, TOMLRef>;
		return TOMLTableMember<Const>{ Unicode::FromUTF8(m_it->first), Reference{ m_root, &(m_it->second) } };
	}

	template <bool Const>
	TOMLTableView<Const>::TOMLTableView(const RootPointer root, TableType* table) noexcept
		: m_root{ root }
		, m_table{ table } {}

	template <bool Const>
	typename TOMLTableView<Const>::Iterator TOMLTableView<Const>::begin() const
	{
		return (m_table ? Iterator{ m_root, m_table->begin() } : Iterator{});
	}

	template <bool Const>
	typename TOMLTableView<Const>::Iterator TOMLTableView<Const>::end() const
	{
		return (m_table ? Iterator{ m_root, m_table->end() } : Iterator{});
	}

	template <bool Const>
	size_t TOMLTableView<Const>::size() const noexcept
	{
		return (m_table ? m_table->size() : 0);
	}

	template <bool Const>
	bool TOMLTableView<Const>::isEmpty() const noexcept
	{
		return (size() == 0);
	}
}
