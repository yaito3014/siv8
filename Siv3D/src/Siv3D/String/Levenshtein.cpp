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

# include <Siv3D/Common.hpp>
# include <Siv3D/String.hpp>
# include <Siv3D/Array.hpp>
# include <Siv3D/SIMD.hpp>

# if SIV3D_INTRINSIC(SSE)
#	include <ThirdParty/levenshtein-sse/levenshtein-sse.hpp>
# endif

namespace s3d
{
	namespace
	{
		[[nodiscard]]
		static size_t LevenshteinDistance(const StringView s1, const StringView s2) noexcept
		{
			const size_t minSize = s1.size();
			const size_t maxSize = s2.size();

			if (maxSize < minSize)
			{
				return LevenshteinDistance(s2, s1);
			}

			Array<size_t> distances(minSize + 1);

			for (size_t i = 0; i <= minSize; ++i)
			{
				distances[i] = i;
			}

			for (size_t k = 1; k <= maxSize; ++k)
			{
				size_t t = distances[0];

				++distances[0];

				for (size_t i = 1; i <= minSize; ++i)
				{
					const size_t old = distances[i];
					
					if (s1[i - 1] == s2[k - 1])
					{
						distances[i] = t;
					}
					else
					{
						distances[i] = (Min(Min(distances[i - 1], distances[i]), t) + 1);
					}
					
					t = old;
				}
			}

			return distances[minSize];
		}
	}

	////////////////////////////////////////////////////////////////
	//
	//	levenshteinDistanceFrom
	//
	////////////////////////////////////////////////////////////////

	StringView::size_type StringView::levenshteinDistanceFrom(const StringView other) const noexcept
	{
	# if SIV3D_INTRINSIC(SSE)

		return levenshteinSSE::levenshtein(*this, other);

	# else

		return LevenshteinDistance(*this, other);

	# endif
	}

	////////////////////////////////////////////////////////////////
	//
	//	levenshteinDistanceTo
	//
	////////////////////////////////////////////////////////////////

	StringView::size_type StringView::levenshteinDistanceTo(const StringView other) const noexcept
	{
		return levenshteinDistanceFrom(other);
	}

	////////////////////////////////////////////////////////////////
	//
	//	levenshteinDistanceFrom
	//
	////////////////////////////////////////////////////////////////

	String::size_type String::levenshteinDistanceFrom(const StringView other) const noexcept
	{
		return StringView{ *this }.levenshteinDistanceFrom(other);
	}

	////////////////////////////////////////////////////////////////
	//
	//	levenshteinDistanceTo
	//
	////////////////////////////////////////////////////////////////

	String::size_type String::levenshteinDistanceTo(const StringView other) const noexcept
	{
		return StringView{ *this }.levenshteinDistanceFrom(other);
	}
}
