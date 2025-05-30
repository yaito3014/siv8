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
# include "Common.hpp"
# include "Array.hpp"
# include "Concepts.hpp"

namespace s3d
{
	////////////////////////////////////////////////////////////////
	//
	//	DisjointSet
	//
	////////////////////////////////////////////////////////////////

	/// @brief Disjoint Set Union (Union-Find)
	/// @tparam IndexType インデックスの型
	template <Concept::Integral IndexType>
	class DisjointSet
	{
	public:

		/// @brief インデックスの型
		using index_type = IndexType;

		/// @brief 内部のインデックス型
		using signed_index_type = std::make_signed_t<index_type>;

		/// @brief 現在のインデックスの型で保持可能な最大の要素数
		/// @remark 例えば、インデックス型が uint8 の場合は 128 個です。
		static constexpr size_t MaxSize = (static_cast<size_t>(std::numeric_limits<signed_index_type>::max()) + 1);

		////////////////////////////////////////////////////////////////
		//
		//	(constructor)
		//
		////////////////////////////////////////////////////////////////

		/// @brief デフォルトコンストラクタ
		[[nodiscard]]
		DisjointSet() = default;

		/// @brief 要素数を指定して Disjoint Set Union を作成します。
		/// @param n 要素数
		[[nodiscard]]
		explicit DisjointSet(size_t n);

		////////////////////////////////////////////////////////////////
		//
		//	operator bool
		//
		////////////////////////////////////////////////////////////////

		/// @brief 要素を持つかを返します。
		/// @return 要素数が 0 の場合は false, それ以外の場合は true
		[[nodiscard]]
		explicit operator bool() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	isEmpty
		//
		////////////////////////////////////////////////////////////////

		/// @brief 要素が空であるかを返します。
		/// @return 要素数が 0 の場合は true, それ以外の場合は false
		[[nodiscard]]
		bool isEmpty() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	size
		//
		////////////////////////////////////////////////////////////////

		/// @brief 要素数を返します。
		/// @return 要素数
		[[nodiscard]]
		size_t size() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	ssize
		//
		////////////////////////////////////////////////////////////////

		/// @brief 要素数を符号付き整数型で返します。
		/// @return 要素数
		[[nodiscard]]
		isize ssize() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	root
		//
		////////////////////////////////////////////////////////////////

		/// @brief ノード i を含むグループの root を返します。
		/// @param i ノードのインデックス
		/// @return ノード i を含むグループの root
		[[nodiscard]]
		index_type root(index_type i) noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	connected
		//
		////////////////////////////////////////////////////////////////

		/// @brief ノード i とノード k か同じグループに含まれているかを返します。
		/// @param i ノードのインデックス
		/// @param k ノードのインデックス
		/// @return ノード i とノード k か同じグループに含まれている場合 true, それ以外の場合は false
		[[nodiscard]]
		bool connected(index_type i, index_type k) noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	isRoot
		//
		////////////////////////////////////////////////////////////////

		/// @brief ノード i がグループの root であるかを返します。
		/// @param i ノードのインデックス
		/// @return ノード i がグループの root である場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isRoot(index_type i) const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	merge
		//
		////////////////////////////////////////////////////////////////

		/// @brief ノード i を含むグループとノード k を含むグループを併合します。
		/// @param i ノードのインデックス
		/// @param k ノードのインデックス
		/// @return すでに両者が同じグループであった場合 false, それ以外の場合は true
		bool merge(index_type i, index_type k) noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	size
		//
		////////////////////////////////////////////////////////////////

		/// @brief ノード i を含むグループの要素数を返します。
		/// @param i ノードのインデックス
		/// @return ノード i を含むグループの要素数
		[[nodiscard]]
		size_t size(index_type i) noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	countSets
		//
		////////////////////////////////////////////////////////////////

		/// @brief グループの個数を返します。
		/// @return グループの個数
		[[nodiscard]]
		size_t countSets() const noexcept;

		////////////////////////////////////////////////////////////////
		//
		//	reset
		//
		////////////////////////////////////////////////////////////////

		/// @brief グループ関係をすべて解消します。
		void reset() noexcept;

	private:

		// m_parentsOrSize[i] は i の 親,
		// ただし root の場合は (-1 * そのグループに属する要素数)
		Array<signed_index_type> m_parentsOrSize;
	};
}

# include "detail/DisjointSet.ipp"
