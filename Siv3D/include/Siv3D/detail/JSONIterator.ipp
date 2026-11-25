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
	inline JSONIterator::JSONIterator() : m_iterator(), m_index(detail::NotArray) {}

	inline JSONIterator& JSONIterator::operator ++()
	{
		++m_iterator;
		if (m_index != detail::NotArray) ++m_index;
		return *this;
	}

	inline JSONIterator JSONIterator::operator ++(int)
	{
		JSONIterator temp = *this;
		++*this;
		return temp;
	}

	inline JSONIterator& JSONIterator::operator --()
	{
		--m_iterator;
		if (m_index != detail::NotArray) --m_index;
		return *this;
	}

	inline JSONIterator JSONIterator::operator --(int)
	{
		JSONIterator temp = *this;
		--*this;
		return temp;
	}

	inline JSONIterator::JSONIterator(detail::JSONBase::iterator it, std::size_t index)
		: m_iterator(it), m_index(index)
	{}

	inline JSONConstIterator::JSONConstIterator() : m_iterator(), m_index(detail::NotArray) {}

	inline JSONConstIterator::JSONConstIterator(const JSONIterator& itr) : m_iterator(itr.m_iterator), m_index(itr.m_index) {}

	inline JSONConstIterator& JSONConstIterator::operator ++()
	{
		++m_iterator;
		if (m_index != detail::NotArray) ++m_index;
		return *this;
	}

	inline JSONConstIterator JSONConstIterator::operator ++(int)
	{
		JSONConstIterator temp = *this;
		++*this;
		return temp;
	}

	inline JSONConstIterator& JSONConstIterator::operator --()
	{
		--m_iterator;
		if (m_index != detail::NotArray) --m_index;
		return *this;
	}

	inline JSONConstIterator JSONConstIterator::operator --(int)
	{
		JSONConstIterator temp = *this;
		--*this;
		return temp;
	}

	inline JSONConstIterator::JSONConstIterator(detail::JSONBase::const_iterator it, std::size_t index)
		: m_iterator(it), m_index(index)
	{}
}
