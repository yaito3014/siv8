//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2024 Ryo Suzuki
//	Copyright (c) 2016-2024 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include "Siv3DTest.hpp"

constexpr int32 operator""_i32(unsigned long long x) { return x; }
constexpr int64 operator""_i64(unsigned long long x) { return x; }
constexpr uint32 operator""_u32(unsigned long long x) { return x; }
constexpr uint64 operator""_u64(unsigned long long x) { return x; }

TEST_CASE("Step.step(n)")
{
	{
		std::vector<int32> v;

		for (auto i : step(3_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ 0, 1, 2 });
	}

	{
		std::vector<int32> v;

		for (auto i : step(0_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK(v.empty());
	}

	{
		std::vector<uint64> v;

		for (auto i : step(3_u64))
		{
			static_assert(std::is_same_v<decltype(i), uint64>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<uint64>{ 0, 1, 2 });
	}

	{
		std::vector<uint64> v;

		for (auto i : step<uint64>(3))
		{
			static_assert(std::is_same_v<decltype(i), uint64>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<uint64>{ 0, 1, 2 });
	}

	{
		for (auto i : step<char>('A'))
		{
			static_assert(std::is_same_v<decltype(i), char>);
		}
	}
}

TEST_CASE("Step.step(a, n, step)")
{
	{
		std::vector<int32> v;

		for (auto i : step(10_i32, 4_i32, 5))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ 10, 15, 20, 25 });
	}

	{
		std::vector<int32> v;

		for (auto i : step(-10_i32, 4_i32, 5))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ -10, -5, 0, 5 });
	}

	{
		std::vector<int32> v;

		for (auto i : step(10_i32, 4_i32, -5))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ 10, 5, 0, -5 });
	}
}

TEST_CASE("Step.step_backward(n)")
{
	{
		std::vector<int32> v;

		for (auto i : step_backward(3_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ 2, 1, 0 });
	}

	{
		std::vector<int32> v;

		for (auto i : step_backward(0_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK(v.empty());
	}

	{
		std::vector<uint64> v;

		for (auto i : step_backward(3_u64))
		{
			static_assert(std::is_same_v<decltype(i), uint64>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<uint64>{ 2, 1, 0 });
	}

	{
		std::vector<uint64> v;

		for (auto i : step_backward<uint64>(3))
		{
			static_assert(std::is_same_v<decltype(i), uint64>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<uint64>{ 2, 1, 0 });
	}

	{
		for (auto i : step_backward<char>('A'))
		{
			static_assert(std::is_same_v<decltype(i), char>);
		}
	}
}

TEST_CASE("Step.Range(n)")
{
	{
		std::vector<int32> v;

		for (auto i : Range(0_i32, 3_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ 0, 1, 2, 3 });
	}

	{
		std::vector<int32> v;

		for (auto i : Range(-2_i32, 2_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ -2, -1, 0, 1, 2 });
	}

	{
		std::vector<int32> v;

		for (auto i : Range(0_i32, 0_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<int32>{ 0 });
	}

	{
		std::vector<int32> v;

		for (auto i : Range(0_i32, -1_i32))
		{
			static_assert(std::is_same_v<decltype(i), int32>);
			v.push_back(i);
		}

		CHECK(v.empty());
	}

	{
		std::vector<uint64> v;

		for (auto i : Range(0_u64, 3_u64))
		{
			static_assert(std::is_same_v<decltype(i), uint64>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<uint64>{ 0, 1, 2, 3 });
	}

	{
		std::vector<uint64> v;

		for (auto i : Range<uint64>(0, 3))
		{
			static_assert(std::is_same_v<decltype(i), uint64>);
			v.push_back(i);
		}

		CHECK_EQ(v, std::vector<uint64>{ 0, 1, 2, 3 });
	}

	{
		std::string s;

		for (auto i : Range<char>('A', 'F'))
		{
			static_assert(std::is_same_v<decltype(i), char>);
			s.push_back(i);
		}

		CHECK_EQ(s, std::string{ "ABCDEF" });
	}
}
