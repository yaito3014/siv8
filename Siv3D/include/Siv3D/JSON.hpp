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
# include <compare>
# include <typeinfo>
# include "Common.hpp"
# include "Array.hpp"
# include "String.hpp"
# include "Blob.hpp"
# include "Error.hpp"
# include "Optional.hpp"
# include "Result.hpp"
# include "IReader.hpp"
# include "IntToString.hpp"
# include "JSONValueType.hpp"
# include "JSONParseErrorReason.hpp"
# include "PredefinedYesNo.hpp"
// Keep the underlying library's literals out of the global namespace.
# ifndef JSON_USE_GLOBAL_UDLS
# define JSON_USE_GLOBAL_UDLS 0
# endif
SIV3D_DISABLE_MSVC_WARNINGS_PUSH(26819)
# include <ThirdParty/nlohmann/json.hpp>
SIV3D_DISABLE_MSVC_WARNINGS_POP()

namespace s3d
{
	namespace detail
	{
		template <bool Const, class T>
		using maybe_const = std::conditional_t<Const, const T, T>;
	}

	class JSON;

	class JSONIterator;

	class JSONConstIterator;

	class JSONRef;

	class JSONConstRef;

	template <bool>
	class JSONArrayView;

	template <bool>
	class JSONArrayViewIterator;

	namespace detail
	{
		using JSONBase = nlohmann::ordered_json;

		[[noreturn]]
		void ThrowJSONGetError(const char* type, std::string_view s);

		template <class Type>
		concept JSONLike = std::same_as<Type, JSON> || std::same_as<Type, JSONRef> || std::same_as<Type, JSONConstRef>;

		template <class Derived>
		class JSONMixin
		{
		private:

			const JSONBase& getRef() const noexcept;

		public:

			using const_iterator = JSONConstIterator;

			/// @brief 値の型を返します。
			/// @return 値の型
			[[nodiscard]]
			JSONValueType getType() const noexcept;

			/// @brief 値が null であるかを返します。
			/// @return 値が null である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isNull() const noexcept;

			/// @brief 値がオブジェクトであるかを返します。
			/// @return 値がオブジェクトである場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isObject() const noexcept;

			/// @brief 値が配列であるかを返します。
			/// @return 値が配列である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isArray() const noexcept;

			/// @brief 値が文字列であるかを返します。
			/// @return 値が文字列である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isString() const noexcept;

			/// @brief 値がプリミティブ型 (null, bool, 数値, 文字列, バイナリデータ) であるかを返します。
			/// @return 値がプリミティブ型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isPrimitive() const noexcept;

			/// @brief 値が bool であるかを返します。
			/// @return 値が bool である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isBool() const noexcept;

			/// @brief 値が数値 (符号付き整数、符号なし整数、浮動小数点数) であるかを返します。
			/// @return 値が数値である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isNumber() const noexcept;

			/// @brief 値が整数 (符号付き整数または符号なし整数) であるかを返します。
			/// @return 値が整数である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isInt() const noexcept;

			/// @brief 値が符号なし整数であるかを返します。
			/// @return 値が符号なし整数である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isUint() const noexcept;

			/// @brief 値が浮動小数点数であるかを返します。
			/// @return 値が浮動小数点数である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isFloat() const noexcept;

			/// @brief 値がバイナリデータであるかを返します。
			/// @return 値がバイナリデータである場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isBinary() const noexcept;

			/// @brief 値が不正であるかを返します。
			/// @return 値が不正である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isInvalid() const noexcept;

			/// @brief 値が null でも不正でもないかを返します。
			/// @return 値が null でも不正でもない場合 true, それ以外の場合は false
			[[nodiscard]]
			explicit operator bool() const noexcept;

			/// @brief 値がオブジェクトで、指定したキーの要素を持つかを返します。
			/// @param key キー (UTF-8)
			/// @return 値がオブジェクトでキーの要素を持つ場合 true, それ以外の場合は false
			/// @remark `contains()` と同じです。
			[[nodiscard]]
			bool hasElement(std::string_view key) const;

			/// @brief 値がオブジェクトで、指定したキーの要素を持つかを返します。
			/// @param key キー
			/// @return 値がオブジェクトでキーの要素を持つ場合 true, それ以外の場合は false
			/// @remark `contains()` と同じです。
			[[nodiscard]]
			bool hasElement(StringView key) const;

			/// @brief 値がオブジェクトで、指定したキーの要素を持つかを返します。
			/// @param key キー (UTF-8)
			/// @return 値がオブジェクトでキーの要素を持つ場合 true, それ以外の場合は false
			[[nodiscard]]
			bool contains(std::string_view key) const;

			/// @brief 値がオブジェクトで、指定したキーの要素を持つかを返します。
			/// @param key キー
			/// @return 値がオブジェクトでキーの要素を持つ場合 true, それ以外の場合は false
			[[nodiscard]]
			bool contains(StringView key) const;

			/// @brief 配列の要素数またはオブジェクトのキーの数を返します。
			/// @return 配列の要素数またはオブジェクトのキーの数。null は 0、その他の値は 1
			[[nodiscard]]
			std::size_t size() const noexcept;

			/// @brief 値が文字列の場合、String として返します。
			/// @return 値
			/// @throw Error 値が文字列でない場合
			[[nodiscard]]
			String getString() const;

			/// @brief 値が文字列の場合、UTF-8 文字列として返します。
			/// @return 値
			/// @throw Error 値が文字列でない場合
			[[nodiscard]]
			std::string getUTF8() const;

			/// @brief 値がバイナリデータの場合、そのバイナリデータを返します。
			/// @return バイナリデータ
			/// @throw Error 値がバイナリデータでない場合
			[[nodiscard]]
			Blob getBinary() const;

			/// @brief 値を Type 型として取得します。
			/// @tparam Type 値の型
			/// @return 値
			/// @throw Error 値を Type 型に変換できない場合
			/// @remark 数値の範囲は検査しません。整数への変換は小数部分を切り捨てます。
			/// @pre 浮動小数点数から整数への変換では、切り捨て後の値が変換先の範囲内である必要があります。
			template <class Type>
			[[nodiscard]]
			Type get() const;

			/// @brief 値を Type 型として取得します。取得できなかった場合は defaultValue を返します。
			/// @tparam Type 値の型
			/// @tparam U 取得できなかった場合に返す値の型
			/// @param defaultValue 取得できなかった場合に返す値
			/// @return 値
			/// @remark 数値変換の規則と事前条件は `get()` と同じです。範囲検査には使用できません。
			template <class Type, class U>
				requires std::constructible_from<Type, U>
			[[nodiscard]]
			Type getOr(U&& defaultValue) const;

			/// @brief 値を Type 型として取得します。
			/// @tparam Type 値の型
			/// @return 値。取得できなかった場合は none
			/// @remark 数値変換の規則と事前条件は `get()` と同じです。範囲検査には使用できません。
			template <class Type>
			[[nodiscard]]
			Optional<Type> getOpt() const;

			/// @brief オブジェクトまたは配列の要素の先頭を指す読み取り専用のイテレータを返します。
			/// @return オブジェクトまたは配列の要素の先頭を指すイテレータ
			[[nodiscard]]
			const_iterator begin() const noexcept;

			/// @brief オブジェクトまたは配列の要素の終端位置を指す読み取り専用のイテレータを返します。
			/// @return オブジェクトまたは配列の要素の終端位置を指すイテレータ
			[[nodiscard]]
			const_iterator end() const noexcept;

			/// @brief オブジェクトまたは配列の要素の先頭を指す読み取り専用のイテレータを返します。
			/// @return オブジェクトまたは配列の要素の先頭を指すイテレータ
			[[nodiscard]]
			const_iterator cbegin() const noexcept;

			/// @brief オブジェクトまたは配列の要素の終端位置を指す読み取り専用のイテレータを返します。
			/// @return オブジェクトまたは配列の要素の終端位置を指すイテレータ
			[[nodiscard]]
			const_iterator cend() const noexcept;

			/// @brief オブジェクトの指定したキーの要素を読み取り専用で参照します。
			/// @param key キー (UTF-8)
			/// @return 要素への参照
			/// @throw Error 値がオブジェクトでない場合、またはキーがない場合
			[[nodiscard]]
			JSONConstRef operator [](std::string_view key) const;

			/// @brief オブジェクトの指定したキーの要素を読み取り専用で参照します。
			/// @param key キー
			/// @return 要素への参照
			/// @throw Error 値がオブジェクトでない場合、またはキーがない場合
			[[nodiscard]]
			JSONConstRef operator [](StringView key) const;

			/// @brief 配列の指定したインデックスの要素を読み取り専用で参照します。
			/// @param index インデックス
			/// @return 要素への参照
			/// @throw Error 値が配列でない場合、またはインデックスが範囲外の場合
			[[nodiscard]]
			JSONConstRef operator [](std::size_t index) const;

			/// @brief 配列の要素を読み取り専用で参照するビューを返します。
			/// @return ビュー
			/// @throw Error 値が配列でない場合
			[[nodiscard]]
			JSONArrayView<true> arrayView() const;

			/// @brief JSON ポインタとプリミティブ型の値のペアからなるオブジェクトに変換した結果を返します。
			/// @return オブジェクト
			/// @remark 空の配列とオブジェクトは null になり、`unflatten()` では復元できません。
			[[nodiscard]]
			JSON flatten() const;

			/// @brief JSON ポインタと値のオブジェクトから階層を復元します。
			/// @return 復元した値
			/// @remark 空のコンテナは null になり、数字のキーは配列のインデックスと解釈されることがあります。
			[[nodiscard]]
			JSON unflatten() const;

			/// @brief 値を JSON 文字列にフォーマットした結果を返します。
			/// @param indent インデントの文字 (空白またはタブ)
			/// @param spaceCount インデントの文字数 (INT_MAX 以下)
			/// @param ensureAscii ASCII 文字のみにエンコードするか
			/// @return フォーマットした JSON 文字列
			[[nodiscard]]
			String format(char32 indent = U' ', std::size_t spaceCount = 2, EnsureAscii ensureAscii = EnsureAscii::No) const;

			/// @brief 値を UTF-8 の JSON 文字列にフォーマットした結果を返します。
			/// @param indent インデントの文字 (空白またはタブ)
			/// @param spaceCount インデントの文字数 (INT_MAX 以下)
			/// @param ensureAscii ASCII 文字のみにエンコードするか
			/// @return フォーマットした JSON 文字列
			[[nodiscard]]
			std::string formatUTF8(char32 indent = U' ', std::size_t spaceCount = 2, EnsureAscii ensureAscii = EnsureAscii::No) const;

			/// @brief 値を最小限の JSON 文字列にフォーマットした結果を返します。
			/// @param ensureAscii ASCII 文字のみにエンコードするか
			/// @return フォーマットした JSON 文字列
			[[nodiscard]]
			String formatMinified(EnsureAscii ensureAscii = EnsureAscii::No) const;

			/// @brief 値を最小限の UTF-8 の JSON 文字列にフォーマットした結果を返します。
			/// @param ensureAscii ASCII 文字のみにエンコードするか
			/// @return フォーマットした JSON 文字列
			[[nodiscard]]
			std::string formatUTF8Minified(EnsureAscii ensureAscii = EnsureAscii::No) const;

			/// @brief 値を JSON ファイルに保存します。
			/// @param path 保存するファイルのパス
			/// @param indent インデントの文字 (空白またはタブ)
			/// @param spaceCount インデントの文字数 (INT_MAX 以下)
			/// @param ensureAscii ASCII 文字のみにエンコードするか
			/// @return ファイルを開けた場合 true, それ以外の場合は false
			bool save(FilePathView path, char32 indent = U' ', std::size_t spaceCount = 2, EnsureAscii ensureAscii = EnsureAscii::No) const;

			/// @brief 値を最小限の形式で JSON ファイルに保存します。
			/// @param path 保存するファイルのパス
			/// @param ensureAscii ASCII 文字のみにエンコードするか
			/// @return ファイルを開けた場合 true, それ以外の場合は false
			bool saveMinified(FilePathView path, EnsureAscii ensureAscii = EnsureAscii::No) const;

			/// @brief BSON 形式にシリアライズした結果を返します。
			/// @return BSON データ
			/// @pre 値はオブジェクトである必要があります。
			/// @remark INT64_MAX を超える符号なし整数は BSON Timestamp (型コード 0x11) として表現されます。
			[[nodiscard]]
			Blob toBSON() const;

			/// @brief CBOR 形式にシリアライズした結果を返します。
			/// @return CBOR データ
			[[nodiscard]]
			Blob toCBOR() const;

			/// @brief MessagePack 形式にシリアライズした結果を返します。
			/// @return MessagePack データ
			[[nodiscard]]
			Blob toMessagePack() const;

			/// @brief 値への読み取り専用の参照に変換します。
			/// @return 値への参照
			[[nodiscard]]
			operator JSONConstRef() const noexcept SIV3D_LIFETIMEBOUND requires std::same_as<Derived, JSON>;

			// Not lifetimebound: the result refers to the referent, not to this JSONRef.
			/// @brief 参照先への読み取り専用の参照に変換します。
			/// @return 参照先への参照
			[[nodiscard]]
			operator JSONConstRef() const noexcept requires std::same_as<Derived, JSONRef>;

			/// @brief 値を、rhs から作成した JSON の値と比較します。
			/// @param lhs 比較する値
			/// @param rhs 比較する値
			/// @return 等しい場合 true, それ以外の場合は false
			/// @remark 比較の規則は `operator ==(JSONConstRef, JSONConstRef)` と同じです。
			template <class Type>
				requires (!JSONLike<Type>) && (!std::same_as<std::remove_cvref_t<Type>, JSONValueType>) && std::constructible_from<JSONBase, const Type&>
			[[nodiscard]]
			friend bool operator ==(const Derived& lhs, const Type& rhs)
			{
				return (lhs.getRef() == JSONBase(rhs));
			}

			/// @brief 値を文字列に変換します。
			/// @param formatData 文字列バッファ
			/// @param value 値
			/// @remark この関数は Format 用の関数です。通常、ユーザーが直接呼び出す必要はありません。
			friend void Formatter(FormatData& formatData, const Derived& value)
			{
				formatData.string.append(value.format());
			}
		};
	}

	/// @brief 挿入順を保持する JSON の値を所有するクラス。
	/// @remark コピーは値全体を複製します。
	/// @remark `operator []`、`arrayView()`、イテレータから得る要素は値への参照です。値の破棄、代入、要素の追加・削除で無効になることがあります。独立した値が必要な場合は `JSON` にコピーしてください。
	class JSON final : public detail::JSONMixin<JSON>
	{
	private:

		using base_type = detail::JSONMixin<JSON>;

	public:

		/// @brief 基盤ライブラリの JSON 型
		using json_base = detail::JSONBase;

		using iterator = JSONIterator;

		using const_iterator = JSONConstIterator;

		/// @brief null を作成します。
		[[nodiscard]]
		JSON();

		/// @brief 指定した型の空の値を作成します。
		/// @param valueType 値の型
		/// @remark 数値は 0、bool は false、文字列・配列・オブジェクト・バイナリデータは空になります。`JSONValueType::Invalid` を指定すると不正な値になります。
		[[nodiscard]]
		explicit JSON(JSONValueType valueType);

		// JSONValueType is excluded because nlohmann would store it as a number.
		// Defined in the class because MSVC ignores explicit(bool) on an out-of-class definition.
		/// @brief 値から JSON の値を作成します。UTF-32 の文字列・文字列ビュー・文字列ポインタは JSON の文字列になります。
		/// @param value 値
		/// @pre 文字列は型に対応した有効な UTF-8 または UTF-32 である必要があります。
		template <class Type>
			requires (!detail::JSONLike<std::remove_cvref_t<Type>>) && (!std::same_as<std::remove_cvref_t<Type>, JSONValueType>) && std::constructible_from<detail::JSONBase, Type>
		[[nodiscard]]
		explicit(!std::is_convertible_v<Type, detail::JSONBase>) JSON(Type&& value)
			: m_json(std::forward<Type>(value)) {}

		/// @brief 初期化子リストから配列またはオブジェクトを作成します。`JSON{ 42 }` は `[42]`、`JSON(42)` は `42` です。
		/// @param init 要素
		/// @param typeDeduction キーと値のペアのリストをオブジェクトとして推定するか
		/// @param manualType typeDeduction が false の場合の型 (`JSONValueType::Array` または `JSONValueType::Object`)
		/// @throw Error manualType が `JSONValueType::Object` で、init がキーと値のペアのリストでない場合
		[[nodiscard]]
		JSON(json_base::initializer_list_t init, bool typeDeduction = true, JSONValueType manualType = JSONValueType::Array);

		/// @brief イテレータ範囲の各要素から配列を作成します。
		/// @param first 範囲の先頭
		/// @param last 範囲の終端
		/// @pre [first, last) は有効な範囲である必要があります。
		template <class Cpp17InputIterator>
			requires (!std::integral<Cpp17InputIterator>)
		[[nodiscard]]
		JSON(Cpp17InputIterator first, Cpp17InputIterator last);

		[[nodiscard]]
		JSON(const JSON& other);

		[[nodiscard]]
		JSON(JSON&& other) noexcept;

		/// @brief 参照先の値を複製して作成します。
		/// @param ref 複製する値への参照
		[[nodiscard]]
		JSON(JSONRef ref);

		/// @brief 参照先の値を複製して作成します。
		/// @param const_ref 複製する値への参照
		[[nodiscard]]
		JSON(JSONConstRef const_ref);

		/// @brief 値を置き換えます。
		/// @param other 新しい値
		/// @return *this
		/// @remark other は置き換えの前に作成されるため、この値の要素への参照から代入できます。
		JSON& operator =(JSON other) noexcept;

		using base_type::begin;

		/// @brief オブジェクトまたは配列の要素の先頭を指すイテレータを返します。
		/// @return オブジェクトまたは配列の要素の先頭を指すイテレータ
		[[nodiscard]]
		iterator begin() noexcept;

		using base_type::end;

		/// @brief オブジェクトまたは配列の要素の終端位置を指すイテレータを返します。
		/// @return オブジェクトまたは配列の要素の終端位置を指すイテレータ
		[[nodiscard]]
		iterator end() noexcept;

		using base_type::operator [];

		/// @brief オブジェクトの指定したキーの要素を参照します。
		/// @param key キー (UTF-8)
		/// @return 要素への参照
		/// @throw Error 値がオブジェクトでも null でもない場合
		/// @remark キーがなければ null の要素を追加します。値が null の場合は空のオブジェクトにしてから追加します。
		[[nodiscard]]
		JSONRef operator [](std::string_view key);

		/// @brief オブジェクトの指定したキーの要素を参照します。
		/// @param key キー
		/// @return 要素への参照
		/// @throw Error 値がオブジェクトでも null でもない場合
		/// @remark キーがなければ null の要素を追加します。値が null の場合は空のオブジェクトにしてから追加します。
		[[nodiscard]]
		JSONRef operator [](StringView key);

		/// @brief 配列の指定したインデックスの要素を参照します。
		/// @param index インデックス
		/// @return 要素への参照
		/// @throw Error 値が配列でない場合、またはインデックスが範囲外の場合
		[[nodiscard]]
		JSONRef operator [](std::size_t index);

		/// @brief 配列の末尾に要素を追加します。
		/// @param value 追加する要素
		/// @throw Error 値が配列でも null でもない場合
		/// @remark 値が null の場合は配列にしてから追加します。
		/// @pre value をムーブで作成する場合、ムーブ元はこの値を含んでいてはいけません。
		void push_back(JSON value);

		/// @brief 配列の末尾の要素を削除します。
		/// @throw Error 値が配列でない場合、または配列が空の場合
		void pop_back();

		/// @brief 値の型はそのままで、値を空にします。
		/// @remark 配列・オブジェクト・文字列・バイナリデータは空に、数値は 0 に、bool は false になります。
		void clear();

		/// @brief オブジェクトの指定したキーの要素を削除します。
		/// @param key キー (UTF-8)
		/// @throw Error 値がオブジェクトでない場合
		/// @remark キーがない場合は何もしません。
		void erase(std::string_view key);

		/// @brief オブジェクトの指定したキーの要素を削除します。
		/// @param key キー
		/// @throw Error 値がオブジェクトでない場合
		/// @remark キーがない場合は何もしません。
		void erase(StringView key);

		/// @brief 配列の指定したインデックスの要素を削除します。
		/// @param index インデックス
		/// @throw Error 値が配列でない場合、またはインデックスが範囲外の場合
		void erase(std::size_t index);

		using base_type::arrayView;

		/// @brief 配列の要素を参照するビューを返します。
		/// @return ビュー
		/// @throw Error 値が配列でない場合
		[[nodiscard]]
		JSONArrayView<false> arrayView();

		/// @brief 値を交換します。
		/// @param other 交換する値
		void swap(JSON& other) noexcept;

		/// @brief 2 つの値を交換します。
		/// @param lhs 交換する値
		/// @param rhs 交換する値
		friend void swap(JSON& lhs, JSON& rhs) noexcept
		{
			lhs.swap(rhs);
		}

		/// @brief JSON 文字列をパースします。
		/// @param content JSON 文字列 (UTF-8)
		/// @param pathHint エラー位置に使う入力名のヒント
		/// @return パースした値。パースできない場合は不正な値
		/// @remark C / C++ 形式のコメントを許可します。重複キーは最後の値を採用します。
		/// @remark 整数は int64 / uint64、その他の数値は double で保持します。整数の範囲を超える値は double になり精度を失うことがあります。
		[[nodiscard]]
		static JSON Parse(std::string content, FilePathView pathHint = U"");

		/// @brief JSON 文字列をパースします。
		/// @param content JSON 文字列
		/// @param pathHint エラー位置に使う入力名のヒント
		/// @return パースした値。パースできない場合は不正な値
		/// @remark 受理する構文は `Parse(std::string, FilePathView)` と同じです。
		[[nodiscard]]
		static JSON Parse(StringView content, FilePathView pathHint = U"");

		/// @brief JSON 文字列をパースし、結果またはエラー理由を返します。
		/// @param content JSON 文字列 (UTF-8)
		/// @param pathHint エラー位置に使う入力名のヒント。空の場合は `internal string`
		/// @return パースした値。パースできない場合は 1 個のエラー理由の配列
		/// @remark 受理する構文は `Parse()` と同じです。エラーの位置は `入力名:行:列` の形式で、列は UTF-8 のバイト単位です。
		[[nodiscard]]
		static Result<JSON, Array<JSONParseErrorReason>> ParseResult(std::string content, FilePathView pathHint = U"");

		/// @brief JSON 文字列をパースし、結果またはエラー理由を返します。
		/// @param content JSON 文字列
		/// @param pathHint エラー位置に使う入力名のヒント。空の場合は `internal string`
		/// @return パースした値。パースできない場合は 1 個のエラー理由の配列
		/// @remark 受理する構文は `Parse()` と同じです。エラーの位置は `入力名:行:列` の形式で、列は UTF-8 のバイト単位です。
		[[nodiscard]]
		static Result<JSON, Array<JSONParseErrorReason>> ParseResult(StringView content, FilePathView pathHint = U"");

		/// @brief JSON ファイルをロードします。
		/// @param path ファイルパス
		/// @return 読み込んだ値。入力を読み込めない、またはパースできない場合は不正な値
		/// @remark UTF-8 (BOM は任意) と BOM 付き UTF-16LE / UTF-16BE を読み込みます。受理する構文は `Parse()` と同じです。
		[[nodiscard]]
		static JSON Load(FilePathView path);

		/// @brief Reader から JSON をロードします。
		/// @param reader Reader
		/// @return 読み込んだ値。reader が null の場合、入力を読み込めない場合、またはパースできない場合は不正な値
		/// @remark 現在の位置から末尾までを読み込みます。対応する文字コードは `Load(FilePathView)` と同じです。
		[[nodiscard]]
		static JSON Load(std::unique_ptr<IReader> reader);

		/// @brief Reader から JSON をロードします。
		/// @tparam Reader Reader オブジェクトの型
		/// @param reader Reader
		/// @return 読み込んだ値。入力を読み込めない、またはパースできない場合は不正な値
		/// @remark 現在の位置から末尾までを読み込みます。対応する文字コードは `Load(FilePathView)` と同じです。
		template <ReaderObject Reader>
		[[nodiscard]]
		static JSON Load(Reader&& reader);

		/// @brief JSON ファイルをロードし、結果またはエラー理由を返します。
		/// @param path ファイルパス
		/// @return 読み込んだ値。入力を読み込めない、またはパースできない場合は 1 個のエラー理由の配列
		/// @remark エラーの位置の入力名は path です。
		[[nodiscard]]
		static Result<JSON, Array<JSONParseErrorReason>> LoadResult(FilePathView path);

		/// @brief Reader から JSON をロードし、結果またはエラー理由を返します。
		/// @param reader Reader
		/// @return 読み込んだ値。reader が null の場合、入力を読み込めない場合、またはパースできない場合は 1 個のエラー理由の配列
		[[nodiscard]]
		static Result<JSON, Array<JSONParseErrorReason>> LoadResult(std::unique_ptr<IReader> reader);

		/// @brief Reader から JSON をロードし、結果またはエラー理由を返します。
		/// @tparam Reader Reader オブジェクトの型
		/// @param reader Reader
		/// @return 読み込んだ値。入力を読み込めない、またはパースできない場合は 1 個のエラー理由の配列
		template <ReaderObject Reader>
		[[nodiscard]]
		static Result<JSON, Array<JSONParseErrorReason>> LoadResult(Reader&& reader);

		/// @brief BSON 形式のデータからデシリアライズします。
		/// @param bson BSON データ
		/// @param allowExceptions デシリアライズに失敗した場合に例外を発生させるか
		/// @return デシリアライズした値。失敗した場合は不正な値
		/// @throw Error allowExceptions が Yes でデシリアライズに失敗した場合
		[[nodiscard]]
		static JSON FromBSON(const Blob& bson, AllowExceptions allowExceptions = AllowExceptions::No);

		/// @brief CBOR 形式のデータからデシリアライズします。
		/// @param cbor CBOR データ
		/// @param allowExceptions デシリアライズに失敗した場合に例外を発生させるか
		/// @return デシリアライズした値。失敗した場合は不正な値
		/// @throw Error allowExceptions が Yes でデシリアライズに失敗した場合
		[[nodiscard]]
		static JSON FromCBOR(const Blob& cbor, AllowExceptions allowExceptions = AllowExceptions::No);

		/// @brief MessagePack 形式のデータからデシリアライズします。
		/// @param msgpack MessagePack データ
		/// @param allowExceptions デシリアライズに失敗した場合に例外を発生させるか
		/// @return デシリアライズした値。失敗した場合は不正な値
		/// @throw Error allowExceptions が Yes でデシリアライズに失敗した場合
		[[nodiscard]]
		static JSON FromMessagePack(const Blob& msgpack, AllowExceptions allowExceptions = AllowExceptions::No);

		/// @brief 空の配列を作成します。
		/// @return 空の配列
		[[nodiscard]]
		static JSON EmptyArray();

		/// @brief 空のオブジェクトを作成します。
		/// @return 空のオブジェクト
		[[nodiscard]]
		static JSON EmptyObject();

		/// @brief 空のバイナリデータを作成します。
		/// @return 空のバイナリデータ
		[[nodiscard]]
		static JSON EmptyBinary();

		/// @brief 不正な値を作成します。
		/// @return 不正な値
		[[nodiscard]]
		static JSON Invalid();

	private:

		friend base_type;

		friend JSONRef;

		friend JSONConstRef;

		friend JSONIterator;

		friend JSONConstIterator;

		[[nodiscard]]
		const detail::JSONBase& getRefImpl() const noexcept;

		detail::JSONBase m_json;
	};

	/// @brief JSON の値の要素を参照するクラス。
	/// @remark 値を所有しません。コピーは同じ要素を参照し、代入は参照先の値を置き換えます。
	/// @remark 参照先を含む値の破棄、代入、要素の追加・削除で無効になることがあります。独立した値が必要な場合は `JSON` にコピーしてください。
	class JSONRef final : public detail::JSONMixin<JSONRef>
	{
	private:

		using base_type = detail::JSONMixin<JSONRef>;

	public:
		using iterator = JSONIterator;

		[[nodiscard]]
		JSONRef(const JSONRef&) noexcept = default;

		[[nodiscard]]
		JSONRef(JSONRef&&) noexcept = default;

		/// @brief 参照先の値を、other の参照先の値の複製で置き換えます。
		/// @param other 代入する値への参照
		/// @return *this
		JSONRef& operator =(const JSONRef& other);

		/// @brief 参照先の値を、other の参照先の値の複製で置き換えます。
		/// @param other 代入する値への参照
		/// @return *this
		const JSONRef& operator =(const JSONRef& other) const;

		/// @brief 参照先の値を置き換えます。
		/// @param other 新しい値
		/// @return *this
		JSONRef& operator =(const JSON& other);

		/// @brief 参照先の値を置き換えます。
		/// @param other 新しい値
		/// @return *this
		/// @pre other は参照先を含んでいてはいけません。参照先を含む値を代入する場合は、ムーブせずにコピーしてください。
		JSONRef& operator =(JSON&& other);

		/// @brief 参照先の値を置き換えます。
		/// @param other 新しい値
		/// @return *this
		const JSONRef& operator =(const JSON& other) const;

		/// @brief 参照先の値を置き換えます。
		/// @param other 新しい値
		/// @return *this
		/// @pre other は参照先を含んでいてはいけません。参照先を含む値を代入する場合は、ムーブせずにコピーしてください。
		const JSONRef& operator =(JSON&& other) const;

		using base_type::begin;

		/// @brief オブジェクトまたは配列の要素の先頭を指すイテレータを返します。
		/// @return オブジェクトまたは配列の要素の先頭を指すイテレータ
		[[nodiscard]]
		iterator begin() noexcept;

		using base_type::end;

		/// @brief オブジェクトまたは配列の要素の終端位置を指すイテレータを返します。
		/// @return オブジェクトまたは配列の要素の終端位置を指すイテレータ
		[[nodiscard]]
		iterator end() noexcept;

		using base_type::operator [];

		/// @brief オブジェクトの指定したキーの要素を参照します。
		/// @param key キー (UTF-8)
		/// @return 要素への参照
		/// @throw Error 値がオブジェクトでも null でもない場合
		/// @remark キーがなければ null の要素を追加します。値が null の場合は空のオブジェクトにしてから追加します。
		[[nodiscard]]
		JSONRef operator [](std::string_view key);

		/// @brief オブジェクトの指定したキーの要素を参照します。
		/// @param key キー
		/// @return 要素への参照
		/// @throw Error 値がオブジェクトでも null でもない場合
		/// @remark キーがなければ null の要素を追加します。値が null の場合は空のオブジェクトにしてから追加します。
		[[nodiscard]]
		JSONRef operator [](StringView key);

		/// @brief 配列の指定したインデックスの要素を参照します。
		/// @param index インデックス
		/// @return 要素への参照
		/// @throw Error 値が配列でない場合、またはインデックスが範囲外の場合
		[[nodiscard]]
		JSONRef operator [](std::size_t index);

		/// @brief 配列の末尾に要素を追加します。
		/// @param value 追加する要素
		/// @throw Error 値が配列でも null でもない場合
		/// @remark 値が null の場合は配列にしてから追加します。
		/// @pre value をムーブで作成する場合、ムーブ元はこの値を含んでいてはいけません。
		void push_back(JSON value);

		/// @brief 配列の末尾の要素を削除します。
		/// @throw Error 値が配列でない場合、または配列が空の場合
		void pop_back();

		/// @brief 値の型はそのままで、値を空にします。
		/// @remark 配列・オブジェクト・文字列・バイナリデータは空に、数値は 0 に、bool は false になります。
		void clear();

		/// @brief オブジェクトの指定したキーの要素を削除します。
		/// @param key キー (UTF-8)
		/// @throw Error 値がオブジェクトでない場合
		/// @remark キーがない場合は何もしません。
		void erase(std::string_view key);

		/// @brief オブジェクトの指定したキーの要素を削除します。
		/// @param key キー
		/// @throw Error 値がオブジェクトでない場合
		/// @remark キーがない場合は何もしません。
		void erase(StringView key);

		/// @brief 配列の指定したインデックスの要素を削除します。
		/// @param index インデックス
		/// @throw Error 値が配列でない場合、またはインデックスが範囲外の場合
		void erase(std::size_t index);

		using base_type::arrayView;

		/// @brief 配列の要素を参照するビューを返します。
		/// @return ビュー
		/// @throw Error 値が配列でない場合
		[[nodiscard]]
		JSONArrayView<false> arrayView();

		/// @brief 2 つの参照先の値を交換します。
		/// @param lhs 交換する値への参照
		/// @param rhs 交換する値への参照
		/// @remark 参照ではなく、参照先の値を交換します。
		friend void swap(JSONRef lhs, JSONRef rhs) noexcept
		{
			lhs.m_json_ref.swap(rhs.m_json_ref);
		}

	private:

		friend base_type;

		friend JSON;

		friend JSONIterator;

		friend JSONArrayViewIterator<false>;

		[[nodiscard]]
		explicit JSONRef(detail::JSONBase& json_ref);

		[[nodiscard]]
		const detail::JSONBase& getRefImpl() const noexcept;

		detail::JSONBase& m_json_ref;
	};

	/// @brief JSON の値の要素を読み取り専用で参照するクラス。
	/// @remark 値を所有しません。コピーは同じ要素を参照します。
	/// @remark 参照先を含む値の破棄、代入、要素の追加・削除で無効になることがあります。独立した値が必要な場合は `JSON` にコピーしてください。
	class JSONConstRef final : public detail::JSONMixin<JSONConstRef>
	{
	private:

		using base_type = detail::JSONMixin<JSONConstRef>;

	public:

		[[nodiscard]]
		JSONConstRef(const JSONConstRef&) noexcept = default;

		[[nodiscard]]
		JSONConstRef(JSONConstRef&&) noexcept = default;

		JSONConstRef& operator=(const JSONConstRef&) = delete;

		JSONConstRef& operator=(JSONConstRef&&) = delete;

	private:

		friend JSON;

		friend JSONConstIterator;

		template<class>
		friend class detail::JSONMixin;

		template <bool>
		friend class JSONArrayViewIterator;

		friend bool operator ==(JSONConstRef, JSONConstRef) noexcept;

		[[nodiscard]]
		explicit JSONConstRef(const detail::JSONBase& json_ref);

		[[nodiscard]]
		const detail::JSONBase& getRefImpl() const noexcept;

		const detail::JSONBase& m_json_ref;
	};

	/// @brief 2 つの値が等しいかを返します。
	/// @param lhs 比較する値
	/// @param rhs 比較する値
	/// @return 等しい場合 true, それ以外の場合は false
	/// @remark オブジェクトはキーの順序も比較します。数値は型が異なっていても値で比較します。不正な値はそれ自身とも等しくありません。
	[[nodiscard]]
	bool operator ==(JSONConstRef lhs, JSONConstRef rhs) noexcept;

	/// @brief `JSONArrayView` のイテレータ。
	/// @tparam Const 要素を読み取り専用で参照するか
	template <bool Const>
	class JSONArrayViewIterator
	{
	private:

		using iterator_type = std::conditional_t<Const, detail::JSONBase::array_t::const_iterator, detail::JSONBase::array_t::iterator>;

	public:

		using value_type = JSON;

		using difference_type = std::ptrdiff_t;

	private:

		using reference = std::conditional_t<Const, JSONConstRef, JSONRef>;

	public:

		JSONArrayViewIterator() = default;

		JSONArrayViewIterator(JSONArrayViewIterator<!Const> itr) requires Const;

		[[nodiscard]]
		reference operator *() const;

		JSONArrayViewIterator& operator ++();

		JSONArrayViewIterator operator ++(int);

		JSONArrayViewIterator& operator --();

		JSONArrayViewIterator operator --(int);

		JSONArrayViewIterator& operator +=(difference_type n);

		JSONArrayViewIterator& operator -=(difference_type n);

		[[nodiscard]]
		reference operator [](difference_type n) const;

		[[nodiscard]]
		friend JSONArrayViewIterator operator +(JSONArrayViewIterator itr, const difference_type n) noexcept
		{
			return (itr += n);
		}

		[[nodiscard]]
		friend JSONArrayViewIterator operator +(const difference_type n, JSONArrayViewIterator itr) noexcept
		{
			return (itr += n);
		}

		[[nodiscard]]
		friend JSONArrayViewIterator operator -(JSONArrayViewIterator itr, const difference_type n) noexcept
		{
			return (itr -= n);
		}

		[[nodiscard]]
		friend difference_type operator -(const JSONArrayViewIterator& lhs, const JSONArrayViewIterator& rhs) noexcept
		{
			return (lhs.m_iterator - rhs.m_iterator);
		}

		/// @brief 要素の値を `JSON` へムーブします。
		/// @param itr 要素を指すイテレータ
		/// @return ムーブした値
		[[nodiscard]]
		friend JSON iter_move(const JSONArrayViewIterator& itr) noexcept requires (!Const)
		{
			return JSON(std::move(*itr.m_iterator));
		}

		[[nodiscard]]
		friend bool operator ==(const JSONArrayViewIterator&, const JSONArrayViewIterator&) noexcept = default;

		[[nodiscard]]
		friend auto operator <=>(const JSONArrayViewIterator&, const JSONArrayViewIterator&) noexcept = default;

	private:

		template <bool>
		friend class JSONArrayView;

		template <bool>
		friend class JSONArrayViewIterator;

		explicit JSONArrayViewIterator(iterator_type itr);

		iterator_type m_iterator = iterator_type();
	};

	/// @brief JSON の配列の要素を参照するランダムアクセスのビュー。
	/// @tparam Const 要素を読み取り専用で参照するか
	/// @remark 要素は `JSONRef` (Const が true の場合は `JSONConstRef`) として参照します。Const が false の場合、比較関数を指定した `std::ranges::sort` などで要素を並べ替えられます。
	/// @remark 配列を含む値の破棄、代入、配列への要素の追加・削除で、ビューとイテレータは無効になることがあります。
	template <bool Const>
	class JSONArrayView : public std::ranges::view_interface<JSONArrayView<Const>>
	{
	public:

		using iterator = JSONArrayViewIterator<Const>;

		using const_iterator = JSONArrayViewIterator<true>;

		JSONArrayView(JSONArrayView<!Const>) requires Const;

		/// @brief 配列の先頭を指すイテレータを返します。
		/// @return 配列の先頭を指すイテレータ
		iterator begin() noexcept requires (!Const);

		/// @brief 配列の終端位置を指すイテレータを返します。
		/// @return 配列の終端位置を指すイテレータ
		iterator end() noexcept requires (!Const);

		/// @brief 配列の先頭を指す読み取り専用のイテレータを返します。
		/// @return 配列の先頭を指すイテレータ
		const_iterator begin() const noexcept;

		/// @brief 配列の終端位置を指す読み取り専用のイテレータを返します。
		/// @return 配列の終端位置を指すイテレータ
		const_iterator end() const noexcept;

		/// @brief 配列の要素数を返します。
		/// @return 配列の要素数
		std::size_t size() const noexcept;

	private:

		friend JSON;

		friend JSONRef;

		template <bool>
		friend class JSONArrayView;

		template <class>
		friend class detail::JSONMixin;

		explicit JSONArrayView(detail::maybe_const<Const, detail::JSONBase>& json);

		std::reference_wrapper<detail::maybe_const<Const, detail::JSONBase::array_t>> m_array;
	};

	/// @brief 値から JSON の値を作成します。
	/// @tparam Type 値の型
	/// @param value 値
	/// @return 作成した値
	template <class Type>
		requires std::constructible_from<JSON, Type>
	[[nodiscard]]
	JSON ToJSON(Type&& value);

	/// @brief JSON の値を Type 型として取得します。
	/// @tparam Type 値の型
	/// @param json JSON の値
	/// @return 値
	/// @throw Error 値を Type 型に変換できない場合
	/// @remark 数値変換の規則と事前条件は `get()` と同じです。
	template <class Type>
	[[nodiscard]]
	Type FromJSON(JSONConstRef json);

	/// @brief JSON の値を Type 型として取得します。変換できない場合は defaultValue を返します。
	/// @tparam Type 値の型
	/// @param json JSON の値
	/// @param defaultValue 変換できない場合の既定値
	/// @return 取得した値または既定値
	/// @remark 数値変換の規則と事前条件は `get()` と同じです。
	template <class Type>
		requires (not std::is_reference_v<Type>)
	[[nodiscard]]
	Type FromJSONOr(JSONConstRef json, Type&& defaultValue);

	/// @brief JSON の値を Type 型として取得します。変換できない場合は defaultValue を返します。
	/// @tparam Type 値の型
	/// @param json JSON の値
	/// @param defaultValue 変換できない場合の既定値
	/// @return 取得した値または既定値
	/// @remark 数値変換の規則と事前条件は `get()` と同じです。
	template <class Type>
	[[nodiscard]]
	Type FromJSONOr(JSONConstRef json, const Type& defaultValue);

	/// @brief JSON の値を Type 型として取得します。
	/// @tparam Type 値の型
	/// @param json JSON の値
	/// @return 取得した値。変換できない場合は none
	/// @remark 数値変換の規則と事前条件は `get()` と同じです。
	template <class Type>
	[[nodiscard]]
	Optional<Type> FromJSONOpt(JSONConstRef json);

	inline namespace Literals
	{
		inline namespace JSONLiterals
		{
			/// @brief JSON 文字列リテラルをパースします。
			/// @param str JSON 文字列リテラル
			/// @param length JSON 文字列リテラルの長さ
			/// @return パースした値。パースできない場合は不正な値
			[[nodiscard]]
			JSON operator ""_json(const char* str, std::size_t length);

			/// @brief JSON 文字列リテラルをパースします。
			/// @param str JSON 文字列リテラル
			/// @param length JSON 文字列リテラルの長さ
			/// @return パースした値。パースできない場合は不正な値
			[[nodiscard]]
			JSON operator ""_json(const char32_t* str, std::size_t length);
		}
	}
}

# define JSONSerializer nlohmann::adl_serializer

template <std::size_t N>
struct JSONSerializer<s3d::char32[N]>
{
	static void to_json(s3d::JSON::json_base& j, const s3d::char32(&value)[N])
	{
		j = s3d::JSON::json_base(s3d::Unicode::ToUTF8(value));
	}
};

template <>
struct JSONSerializer<s3d::StringView>
{
	static void to_json(s3d::JSON::json_base& j, const s3d::StringView& value);
};

template <>
struct JSONSerializer<s3d::String>
{
	static void to_json(s3d::JSON::json_base& j, const s3d::String& value);

	static void from_json(const s3d::JSON::json_base& j, s3d::String& value);
};

template <>
struct JSONSerializer<std::u32string_view>
{
	static void to_json(s3d::JSON::json_base& j, std::u32string_view value);
};

template <>
struct JSONSerializer<std::u32string>
{
	static void to_json(s3d::JSON::json_base& j, const std::u32string& value);

	static void from_json(const s3d::JSON::json_base& j, std::u32string& value);
};

template <>
struct JSONSerializer<const s3d::char32*>
{
	static void to_json(s3d::JSON::json_base& j, const s3d::char32* value);
};

template <>
struct JSONSerializer<s3d::char32*> : JSONSerializer<const s3d::char32*> {};

template <bool Const>
inline constexpr bool std::ranges::enable_borrowed_range<s3d::JSONArrayView<Const>> = true;

template <template <class> class TQual, template <class> class UQual>
struct std::basic_common_reference<s3d::JSONConstRef, s3d::JSON, TQual, UQual>
{
	using type = s3d::JSONConstRef;
};

template <template <class> class TQual, template <class> class UQual>
struct std::basic_common_reference<s3d::JSON, s3d::JSONConstRef, TQual, UQual>
{
	using type = s3d::JSONConstRef;
};

# include "detail/JSON.ipp"
