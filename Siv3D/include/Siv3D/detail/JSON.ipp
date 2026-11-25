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
		template <class Derived>
		template <class Type>
		Type JSONMixin<Derived>::get() const
		{
			try
			{
				return getRef().template get<Type>();
			}
			catch (const JSONBase::exception&)
			{
				ThrowJSONGetError(typeid(Type).name(), getRef().dump(-1, ' ', false, JSONBase::error_handler_t::replace));
			}
		}

		template <class Derived>
		template <class Type, class U>
			requires std::constructible_from<Type, U>
		Type JSONMixin<Derived>::getOr(U&& defaultValue) const
		{
			try
			{
				return getRef().template get<Type>();
			}
			catch (const JSONBase::exception&)
			{
				return Type(std::forward<U>(defaultValue));
			}
		}

		template <class Derived>
		template <class Type>
		Optional<Type> JSONMixin<Derived>::getOpt() const
		{
			try
			{
				return getRef().template get<Type>();
			}
			catch (const JSONBase::exception&)
			{
				return none;
			}
		}

		template <class Derived>
		inline const JSONBase& JSONMixin<Derived>::getRef() const noexcept
		{
			return static_cast<const Derived&>(*this).getRefImpl();
		}

		extern template class JSONMixin<JSON>;

		extern template class JSONMixin<JSONRef>;

		extern template class JSONMixin<JSONConstRef>;
	}

	inline const detail::JSONBase& JSON::getRefImpl() const noexcept
	{
		return m_json;
	}

	inline JSONRef::JSONRef(detail::JSONBase& json_ref)
		: m_json_ref{ json_ref } {}

	inline const detail::JSONBase& JSONRef::getRefImpl() const noexcept
	{
		return m_json_ref;
	}

	inline JSONConstRef::JSONConstRef(const detail::JSONBase& json_ref)
		: m_json_ref{ json_ref } {}

	inline const detail::JSONBase& JSONConstRef::getRefImpl() const noexcept
	{
		return m_json_ref;
	}

	template <bool Const>
	JSONArrayViewIterator<Const>::JSONArrayViewIterator(JSONArrayViewIterator<!Const> itr) requires Const
		: m_iterator{ itr.m_iterator } {}

	template <bool Const>
	typename JSONArrayViewIterator<Const>::reference JSONArrayViewIterator<Const>::operator *() const
	{
		return reference{ *m_iterator };
	}

	template <bool Const>
	JSONArrayViewIterator<Const>& JSONArrayViewIterator<Const>::operator ++()
	{
		++m_iterator;
		return *this;
	}

	template <bool Const>
	JSONArrayViewIterator<Const> JSONArrayViewIterator<Const>::operator ++(int)
	{
		JSONArrayViewIterator temp = *this;
		++*this;
		return temp;
	}

	template <bool Const>
	JSONArrayViewIterator<Const>& JSONArrayViewIterator<Const>::operator --()
	{
		--m_iterator;
		return *this;
	}

	template <bool Const>
	JSONArrayViewIterator<Const> JSONArrayViewIterator<Const>::operator --(int)
	{
		JSONArrayViewIterator temp = *this;
		--*this;
		return temp;
	}

	template <bool Const>
	JSONArrayViewIterator<Const>& JSONArrayViewIterator<Const>::operator +=(const difference_type n)
	{
		m_iterator += n;
		return *this;
	}

	template <bool Const>
	JSONArrayViewIterator<Const>& JSONArrayViewIterator<Const>::operator -=(const difference_type n)
	{
		m_iterator -= n;
		return *this;
	}

	template <bool Const>
	typename JSONArrayViewIterator<Const>::reference JSONArrayViewIterator<Const>::operator [](const difference_type n) const
	{
		return reference{ m_iterator[n] };
	}

	template <bool Const>
	JSONArrayViewIterator<Const>::JSONArrayViewIterator(iterator_type itr)
		: m_iterator{ itr } {}

	template <bool Const>
	JSONArrayView<Const>::JSONArrayView(JSONArrayView<!Const> other) requires Const
		: m_array{ other.m_array } {}

	template <bool Const>
	typename JSONArrayView<Const>::iterator JSONArrayView<Const>::begin() noexcept requires (!Const)
	{
		return iterator{ m_array.get().begin() };
	}

	template <bool Const>
	typename JSONArrayView<Const>::iterator JSONArrayView<Const>::end() noexcept requires (!Const)
	{
		return iterator{ m_array.get().end() };
	}

	template <bool Const>
	typename JSONArrayView<Const>::const_iterator JSONArrayView<Const>::begin() const noexcept
	{
		return const_iterator{ m_array.get().begin() };
	}

	template <bool Const>
	typename JSONArrayView<Const>::const_iterator JSONArrayView<Const>::end() const noexcept
	{
		return const_iterator{ m_array.get().end() };
	}

	template <bool Const>
	std::size_t JSONArrayView<Const>::size() const noexcept
	{
		return m_array.get().size();
	}

	template <bool Const>
	JSONArrayView<Const>::JSONArrayView(detail::maybe_const<Const, detail::JSONBase>& json)
		: m_array{ json.template get_ref<detail::maybe_const<Const, detail::JSONBase::array_t>&>() } {}

	template <class Cpp17InputIterator>
		requires (!std::integral<Cpp17InputIterator>)
	JSON::JSON(Cpp17InputIterator first, Cpp17InputIterator last)
		: m_json(typename detail::JSONBase::array_t(first, last))
	{}

	template <ReaderObject Reader>
	JSON JSON::Load(Reader&& reader)
	{
		return Load(std::make_unique<Reader>(std::move(reader)));
	}

	template <ReaderObject Reader>
	Result<JSON, Array<JSONParseErrorReason>> JSON::LoadResult(Reader&& reader)
	{
		return LoadResult(std::make_unique<Reader>(std::move(reader)));
	}

	template <class Type>
		requires std::constructible_from<JSON, Type>
	JSON ToJSON(Type&& value)
	{
		return JSON(std::forward<Type>(value));
	}

	template <class Type>
	Type FromJSON(const JSONConstRef json)
	{
		return json.get<Type>();
	}

	template <class Type>
		requires (not std::is_reference_v<Type>)
	Type FromJSONOr(const JSONConstRef json, Type&& defaultValue)
	{
		return json.getOr<Type>(std::forward<Type>(defaultValue));
	}

	template <class Type>
	Type FromJSONOr(const JSONConstRef json, const Type& defaultValue)
	{
		return json.getOr<Type>(defaultValue);
	}

	template <class Type>
	Optional<Type> FromJSONOpt(const JSONConstRef json)
	{
		return json.getOpt<Type>();
	}
}

