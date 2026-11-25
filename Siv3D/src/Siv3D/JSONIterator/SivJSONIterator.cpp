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

# include <Siv3D/JSONIterator.hpp>

namespace s3d
{
	JSONIterator::JSONItemRef::operator JSONItem() const
	{
		return JSONItem{ key, JSON{ value } };
	}

	String JSONIterator::key() const
	{
		if (m_index == detail::NotArray)
		{
			return Unicode::FromUTF8(m_iterator.key());
		}
		else
		{
			return ToString(m_index);
		}
	}

	JSONRef JSONIterator::value() const
	{
		return JSONRef{ m_iterator.value() };
	}

	JSONIterator::JSONItemRef JSONIterator::operator *() const
	{
		return JSONItemRef{ key(), value() };
	}

	JSONConstIterator::JSONItemConstRef::operator JSONItem() const
	{
		return JSONItem{ key, JSON{ value } };
	}

	String JSONConstIterator::key() const
	{
		if (m_index == detail::NotArray)
		{
			return Unicode::FromUTF8(m_iterator.key());
		}
		else
		{
			return ToString(m_index);
		}
	}

	JSONConstRef JSONConstIterator::value() const
	{
		return JSONConstRef{ m_iterator.value() };
	}

	JSONConstIterator::JSONItemConstRef JSONConstIterator::operator *() const
	{
		return JSONItemConstRef{ key(), value() };
	}
}
