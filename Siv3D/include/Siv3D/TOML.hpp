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
# include <initializer_list>
# include <typeinfo>
# include <utility>
# include "Common.hpp"
# include "String.hpp"
# include "Array.hpp"
# include "HashMap.hpp"
# include "DateTime.hpp"
# include "FormatData.hpp"
# include "Unicode.hpp"
# include "Result.hpp"
# include "IReader.hpp"
# include "TOMLValueType.hpp"
# include "TOMLParseErrorReason.hpp"
# ifndef TOML11_COMPILE_SOURCES
#	define TOML11_COMPILE_SOURCES
# endif
# include <ThirdParty/toml11/toml.hpp>

namespace s3d
{
	class TOML;

	class TOMLRef;

	class TOMLConstRef;

	template <bool Const>
	class TOMLArrayView;

	template <bool Const>
	class TOMLTableView;

	template <bool Const>
	struct TOMLTableMember;

	namespace detail
	{
		using TOMLValue = toml::ordered_value;

		/// @brief TOML の local time
		struct TOMLLocalTime
		{
			/// @brief 時
			uint8  hour = 0;

			/// @brief 分
			uint8  minute = 0;

			/// @brief 秒
			uint8  second = 0;

			/// @brief ミリ秒
			uint16 millisecond = 0;

			/// @brief マイクロ秒
			uint16 microsecond = 0;

			/// @brief ナノ秒
			uint16 nanosecond = 0;

			/// @brief DateTime に変換します。
			/// @return 変換された DateTime
			/// @remark DateTime が保持できないマイクロ秒およびナノ秒の情報は失われます。
			[[nodiscard]]
			DateTime toDateTime() const noexcept;

			/// @brief ミリ秒精度までに切り詰めた DateTime に変換します。
			/// @return 変換された DateTime
			[[nodiscard]]
			DateTime toDateTimeTruncatedToMilliseconds() const noexcept;

			/// @brief toml11 の local time から LocalTime を作成します。
			/// @param localTime toml11 の local time
			/// @return 作成した LocalTime
			[[nodiscard]]
			static TOMLLocalTime FromTOML(const toml::local_time& localTime) noexcept;
		};

		/// @brief TOML の local date-time
		struct TOMLLocalDateTime
		{
			/// @brief 日付
			Date date;

			/// @brief 時刻
			TOMLLocalTime time;

			/// @brief DateTime に変換します。
			/// @return 変換された DateTime
			/// @remark DateTime が保持できないマイクロ秒およびナノ秒の情報は失われます。
			[[nodiscard]]
			DateTime toDateTime() const noexcept;

			/// @brief ミリ秒精度までに切り詰めた DateTime に変換します。
			/// @return 変換された DateTime
			[[nodiscard]]
			DateTime toDateTimeTruncatedToMilliseconds() const noexcept;

			/// @brief toml11 の local date-time から LocalDateTime を作成します。
			/// @param localDateTime toml11 の local date-time
			/// @return 作成した LocalDateTime
			[[nodiscard]]
			static TOMLLocalDateTime FromTOML(const toml::local_datetime& localDateTime) noexcept;
		};

		/// @brief TOML の offset date-time
		struct TOMLOffsetDateTime
		{
			/// @brief ローカル日時
			TOMLLocalDateTime localDateTime;

			/// @brief UTC からのオフセット（分）
			int32 offsetMinutes = 0;

			/// @brief toml11 の offset date-time から OffsetDateTime を作成します。
			/// @param offsetDateTime toml11 の offset date-time
			/// @return 作成した OffsetDateTime
			[[nodiscard]]
			static TOMLOffsetDateTime FromTOML(const toml::offset_datetime& offsetDateTime) noexcept;
		};

		[[nodiscard]]
		Date ToDate(const toml::local_date& date) noexcept;

		[[noreturn]]
		void ThrowTOMLGetError(const char* type, const TOMLValue* root, const TOMLValue& value);

		[[noreturn]]
		void ThrowTOMLParsedGetError(const char* type, const TOMLValue* root, const TOMLValue& value);

		[[noreturn]]
		void ThrowTOMLNotArray(const char* type, const TOMLValue* root, const TOMLValue& value);

		[[noreturn]]
		void ThrowTOMLArrayElementGetError(const char* type, const TOMLValue* root, const TOMLValue& value);

		[[noreturn]]
		void ThrowTOMLIntegerOutOfRange(uint64 value);

		template <class Derived>
		class TOMLMixin
		{
		public:

			/// @brief TOML の local time
			using LocalTime = TOMLLocalTime;

			/// @brief TOML の local date-time
			using LocalDateTime = TOMLLocalDateTime;

			/// @brief TOML の offset date-time
			using OffsetDateTime = TOMLOffsetDateTime;

			/// @brief 値が有効であるかを返します。
			/// @return 値が有効である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isValid() const noexcept;

			/// @brief 値が有効であるかを返します。
			/// @return 値が有効である場合 true, それ以外の場合は false
			[[nodiscard]]
			explicit operator bool() const noexcept;

			/// @brief 値が無効であるかを返します。
			/// @return 値が無効である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isInvalid() const noexcept;

			/// @brief ドキュメントのルートからこの値までのパスを返します。
			/// @return `$["server"]["ports"][0]` の形式のパス。TOML 自身およびそのルートを参照する場合は `$`
			/// @remark ルートから値を探索するため、ドキュメントの大きさに比例する時間がかかります。
			[[nodiscard]]
			String path() const;

			/// @brief 値の型を返します。
			/// @return 値の型
			[[nodiscard]]
			TOMLValueType type() const noexcept;

			/// @brief 値の型を返します。
			/// @return 値の型
			[[nodiscard]]
			TOMLValueType getType() const noexcept;

			/// @brief 値が bool 型であるかを返します。
			/// @return 値が bool 型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isBool() const noexcept;

			/// @brief 値が整数型であるかを返します。
			/// @return 値が整数型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isInt() const noexcept;

			/// @brief 値が浮動小数点数型であるかを返します。
			/// @return 値が浮動小数点数型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isFloat() const noexcept;

			/// @brief 値が文字列型であるかを返します。
			/// @return 値が文字列型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isString() const noexcept;

			/// @brief 値が local date 型であるかを返します。
			/// @return 値が local date 型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isLocalDate() const noexcept;

			/// @brief 値が local time 型であるかを返します。
			/// @return 値が local time 型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isLocalTime() const noexcept;

			/// @brief 値が local date-time 型であるかを返します。
			/// @return 値が local date-time 型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isLocalDateTime() const noexcept;

			/// @brief 値が offset date-time 型であるかを返します。
			/// @return 値が offset date-time 型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isOffsetDateTime() const noexcept;

			/// @brief 値が配列型であるかを返します。
			/// @return 値が配列型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isArray() const noexcept;

			/// @brief 値がテーブル型であるかを返します。
			/// @return 値がテーブル型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isTable() const noexcept;

			/// @brief 値が指定した型であるかを返します。
			/// @param valueType 調べる型
			/// @return 値が指定した型である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool is(TOMLValueType valueType) const noexcept;

			/// @brief テーブルが指定したキーを持っているかを返します。
			/// @param key キー（UTF-8）
			/// @return テーブルが指定したキーを持っている場合 true, それ以外の場合は false
			[[nodiscard]]
			bool contains(const std::string& key) const;

			/// @brief テーブルが指定したキーを持っているかを返します。
			/// @param key キー
			/// @return テーブルが指定したキーを持っている場合 true, それ以外の場合は false
			[[nodiscard]]
			bool contains(StringView key) const;

			/// @brief 配列またはテーブルの要素数を返します。
			/// @return 配列またはテーブルの要素数。配列でもテーブルでもない場合は 0
			[[nodiscard]]
			size_t size() const noexcept;

			/// @brief 値が空の配列であるかを返します。
			/// @return 値が空の配列である場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isEmptyArray() const noexcept;

			/// @brief 値が空のテーブルであるかを返します。
			/// @return 値が空のテーブルである場合 true, それ以外の場合は false
			[[nodiscard]]
			bool isEmptyTable() const noexcept;

			/// @brief テーブルの指定したキーの値を読み取り専用で参照します。
			/// @param key キー（UTF-8）
			/// @return 値への参照
			/// @throw Error テーブルでない場合、または指定したキーが存在しない場合
			[[nodiscard]]
			TOMLConstRef operator [](const std::string& key) const;

			/// @brief テーブルの指定したキーの値を読み取り専用で参照します。
			/// @param key キー
			/// @return 値への参照
			/// @throw Error テーブルでない場合、または指定したキーが存在しない場合
			[[nodiscard]]
			TOMLConstRef operator [](StringView key) const;

			/// @brief 配列の指定したインデックスの値を読み取り専用で参照します。
			/// @param index インデックス
			/// @return 値への参照
			/// @throw Error 配列でない場合、またはインデックスが範囲外の場合
			[[nodiscard]]
			TOMLConstRef operator [](size_t index) const;

			/// @brief テーブルの指定したキーの値を読み取り専用で参照します。
			/// @param key キー（UTF-8）
			/// @return 値への参照
			/// @throw Error テーブルでない場合、または指定したキーが存在しない場合
			[[nodiscard]]
			TOMLConstRef at(const std::string& key) const;

			/// @brief テーブルの指定したキーの値を読み取り専用で参照します。
			/// @param key キー
			/// @return 値への参照
			/// @throw Error テーブルでない場合、または指定したキーが存在しない場合
			[[nodiscard]]
			TOMLConstRef at(StringView key) const;

			/// @brief 配列の指定したインデックスの値を読み取り専用で参照します。
			/// @param index インデックス
			/// @return 値への参照
			/// @throw Error 配列でない場合、またはインデックスが範囲外の場合
			[[nodiscard]]
			TOMLConstRef at(size_t index) const;

			/// @brief 値を String として取得します。
			/// @return 値の文字列
			/// @throw Error 値を String として取得できない場合
			[[nodiscard]]
			String getString() const;

			/// @brief 値を UTF-8 文字列として取得します。
			/// @return 値の UTF-8 文字列
			/// @throw Error 値を UTF-8 文字列として取得できない場合
			[[nodiscard]]
			std::string getUTF8() const;

			/// @brief 値を LocalTime として取得します。
			/// @return 値の LocalTime
			/// @throw Error 値を LocalTime として取得できない場合
			[[nodiscard]]
			LocalTime getLocalTime() const;

			/// @brief 値を Date として取得します。
			/// @return 値の Date
			/// @throw Error 値を Date として取得できない場合
			[[nodiscard]]
			Date getLocalDate() const;

			/// @brief 値を LocalDateTime として取得します。
			/// @return 値の LocalDateTime
			/// @throw Error 値を LocalDateTime として取得できない場合
			[[nodiscard]]
			LocalDateTime getLocalDateTime() const;

			/// @brief 値を OffsetDateTime として取得します。
			/// @return 値の OffsetDateTime
			/// @throw Error 値を OffsetDateTime として取得できない場合
			[[nodiscard]]
			OffsetDateTime getOffsetDateTime() const;

			/// @brief 値を指定した型として取得します。
			/// @tparam Type 取得する型。TOML を指定した場合は値の複製を返します。
			/// @return 取得に成功した場合は値, それ以外の場合は none
			template <class Type>
			[[nodiscard]]
			Optional<Type> getOpt() const;

			/// @brief 値を指定した型として取得します。
			/// @tparam Type 取得する型。TOML を指定した場合は値の複製を返します。
			/// @return 取得した値
			/// @throw Error 値を Type 型として取得できない場合
			template <class Type>
			[[nodiscard]]
			Type get() const;

			/// @brief 値を指定した型として取得します。取得できない場合はデフォルト値を返します。
			/// @tparam Type 取得する型
			/// @tparam U デフォルト値の型
			/// @param defaultValue デフォルト値
			/// @return 取得に成功した場合は値, それ以外の場合はデフォルト値
			template <class Type, class U>
			[[nodiscard]]
			Type getOr(U&& defaultValue) const;

			/// @brief 文字列値を指定した型にパースして取得します。
			/// @tparam Type 取得する型
			/// @return パースに成功した場合は値, それ以外の場合は none
			template <class Type>
			[[nodiscard]]
			Optional<Type> getParsedOpt() const;

			/// @brief 文字列値を指定した型にパースして取得します。
			/// @tparam Type 取得する型
			/// @return パースした値
			/// @throw Error 値が文字列でない場合、またはパースに失敗した場合
			template <class Type>
			[[nodiscard]]
			Type getParsed() const;

			/// @brief 文字列値を指定した型にパースして取得します。取得できない場合はデフォルト値を返します。
			/// @tparam Type 取得する型
			/// @tparam U デフォルト値の型
			/// @param defaultValue デフォルト値
			/// @return パースに成功した場合は値, それ以外の場合はデフォルト値
			template <class Type, class U>
			[[nodiscard]]
			Type getParsedOr(U&& defaultValue) const;

			/// @brief 配列を指定した型の配列として取得します。
			/// @tparam Type 要素の型。TOML を指定した場合は要素の複製を返します。
			/// @return 取得に成功した場合は配列, それ以外の場合は none
			template <class Type>
			[[nodiscard]]
			Optional<Array<Type>> getArrayOpt() const;

			/// @brief 配列を指定した型の配列として取得します。
			/// @tparam Type 要素の型。TOML を指定した場合は要素の複製を返します。
			/// @return 取得した配列
			/// @throw Error 値が配列でない場合、または要素を Type 型として取得できない場合
			template <class Type>
			[[nodiscard]]
			Array<Type> getArray() const;

			/// @brief 配列の要素を読み取り専用で参照するビューを返します。
			/// @return 配列のビュー
			/// @throw Error 値が配列でない場合
			[[nodiscard]]
			TOMLArrayView<true> arrayView() const;

			/// @brief テーブルのメンバを読み取り専用で参照するビューを返します。
			/// @return テーブルのビュー
			/// @throw Error 値がテーブルでない場合
			[[nodiscard]]
			TOMLTableView<true> tableView() const;

			/// @brief 値を TOML 形式の文字列に変換します。
			/// @return TOML 形式の文字列。無効な値の場合は空文字列
			/// @throw Error 無効な値を要素に含む場合
			[[nodiscard]]
			String format() const;

			/// @brief 値を TOML 形式の UTF-8 文字列に変換します。
			/// @return TOML 形式の UTF-8 文字列。無効な値の場合は空文字列
			/// @throw Error 無効な値を要素に含む場合
			[[nodiscard]]
			std::string formatUTF8() const;

			/// @brief 値を文字列にフォーマットします。
			/// @param formatData 文字列バッファ
			/// @param value 値
			/// @throw Error 無効な値を要素に含む場合
			friend void Formatter(FormatData& formatData, const Derived& value)
			{
				formatData.string.append(value.format());
			}

		protected:

			[[nodiscard]]
			const TOMLValue& getValue() const noexcept;

			[[nodiscard]]
			const TOMLValue* getRoot() const noexcept;
		};

		template <class Derived>
		class TOMLMutableMixin : public TOMLMixin<Derived>
		{
		public:

			using TOMLMixin<Derived>::operator [];

			using TOMLMixin<Derived>::at;

			using TOMLMixin<Derived>::arrayView;

			using TOMLMixin<Derived>::tableView;

			/// @brief テーブルの指定したキーの値を参照します。
			/// @param key キー（UTF-8）
			/// @return 値への参照
			/// @throw Error 値がテーブルでも無効な値でもない場合
			/// @remark キーがなければ無効な値を追加します。値が無効な値の場合は空のテーブルにしてから追加します。
			/// @remark テーブルへの追加は、そのテーブルの要素を指す既存の参照とビューを無効にします。代入の右辺は左辺より先に評価されるため、`toml[U"new"] = toml[U"old"];` の右辺の参照は `"new"` の追加で無効になり得ます。`toml[U"new"] = TOML{ toml[U"old"] };` のように複製してから代入してください。
			[[nodiscard]]
			TOMLRef operator [](const std::string& key);

			/// @brief テーブルの指定したキーの値を参照します。
			/// @param key キー
			/// @return 値への参照
			/// @throw Error 値がテーブルでも無効な値でもない場合
			/// @remark キーがなければ無効な値を追加します。値が無効な値の場合は空のテーブルにしてから追加します。
			/// @remark テーブルへの追加は、そのテーブルの要素を指す既存の参照とビューを無効にします。代入の右辺は左辺より先に評価されるため、`toml[U"new"] = toml[U"old"];` の右辺の参照は `"new"` の追加で無効になり得ます。`toml[U"new"] = TOML{ toml[U"old"] };` のように複製してから代入してください。
			[[nodiscard]]
			TOMLRef operator [](StringView key);

			/// @brief 配列の指定したインデックスの値を参照します。
			/// @param index インデックス
			/// @return 値への参照
			/// @throw Error 配列でない場合、またはインデックスが範囲外の場合
			[[nodiscard]]
			TOMLRef operator [](size_t index);

			/// @brief テーブルの指定したキーの値を参照します。
			/// @param key キー（UTF-8）
			/// @return 値への参照
			/// @throw Error テーブルでない場合、または指定したキーが存在しない場合
			[[nodiscard]]
			TOMLRef at(const std::string& key);

			/// @brief テーブルの指定したキーの値を参照します。
			/// @param key キー
			/// @return 値への参照
			/// @throw Error テーブルでない場合、または指定したキーが存在しない場合
			[[nodiscard]]
			TOMLRef at(StringView key);

			/// @brief 配列の指定したインデックスの値を参照します。
			/// @param index インデックス
			/// @return 値への参照
			/// @throw Error 配列でない場合、またはインデックスが範囲外の場合
			[[nodiscard]]
			TOMLRef at(size_t index);

			/// @brief 配列の末尾に要素を追加します。
			/// @param value 追加する値
			/// @throw Error 値が配列でも無効な値でもない場合
			/// @pre value をムーブで作成する場合、ムーブ元はこの値を含んでいてはいけません。
			/// @remark 値が無効な値の場合は空の配列にしてから追加します。追加は、その配列の要素を指す既存の参照とビューを無効にします。
			void push_back(TOML value);

			/// @brief 配列の末尾の要素を削除します。
			/// @throw Error 値が配列でない場合、または配列が空の場合
			void pop_back();

			/// @brief 配列またはテーブルの要素をすべて削除します。
			/// @throw Error 値が配列でもテーブルでもない場合
			void clear();

			/// @brief テーブルの指定したキーの要素を削除します。
			/// @param key キー（UTF-8）
			/// @throw Error 値がテーブルでない場合
			/// @remark キーがない場合は何もしません。
			void erase(const std::string& key);

			/// @brief テーブルの指定したキーの要素を削除します。
			/// @param key キー
			/// @throw Error 値がテーブルでない場合
			/// @remark キーがない場合は何もしません。
			void erase(StringView key);

			/// @brief 配列の指定したインデックスの要素を削除します。
			/// @param index インデックス
			/// @throw Error 値が配列でない場合、またはインデックスが範囲外の場合
			void erase(size_t index);

			/// @brief 配列の要素を参照するビューを返します。
			/// @return 配列のビュー
			/// @throw Error 値が配列でない場合
			[[nodiscard]]
			TOMLArrayView<false> arrayView();

			/// @brief テーブルのメンバを参照するビューを返します。
			/// @return テーブルのビュー
			/// @throw Error 値がテーブルでない場合
			[[nodiscard]]
			TOMLTableView<false> tableView();

		protected:

			[[nodiscard]]
			TOMLValue& getMutableValue() noexcept;

			[[nodiscard]]
			TOMLValue* getMutableRoot() noexcept;
		};
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOML
	//
	////////////////////////////////////////////////////////////////

	/// @brief TOML の値を所有するクラス
	/// @remark コピーは値全体を複製します。要素へのアクセスは TOMLRef または TOMLConstRef を返します。それらはこの値が破棄、ムーブ、または再代入されるか、参照先を含む配列またはテーブルに要素が追加または削除されると無効になります。
	class TOML final : public detail::TOMLMutableMixin<TOML>
	{
	public:

		/// @brief 内部で使用する TOML 値の型
		using Value = detail::TOMLValue;

		/// @brief デフォルトコンストラクタ。無効な値を作成します。
		[[nodiscard]]
		TOML() = default;

		[[nodiscard]]
		TOML(const TOML&) = default;

		[[nodiscard]]
		TOML(TOML&&) = default;

		/// @brief 参照先の値の複製を作成します。
		/// @param value 参照
		[[nodiscard]]
		TOML(TOMLConstRef value);

		/// @brief 参照先の値の複製を作成します。
		/// @param value 参照
		[[nodiscard]]
		TOML(TOMLRef value);

		/// @brief bool 型の値を作成します。
		/// @param value 値
		template <std::same_as<bool> Bool>
		[[nodiscard]]
		TOML(Bool value);

		/// @brief 整数型の値を作成します。
		/// @param value 値
		/// @throw Error value が int64 の範囲外の場合
		template <class Int>
			requires (std::integral<Int> && (not std::same_as<Int, bool>) && (not std::same_as<Int, char>)
				&& (not std::same_as<Int, char8_t>) && (not std::same_as<Int, char16_t>)
				&& (not std::same_as<Int, char32_t>) && (not std::same_as<Int, wchar_t>))
		[[nodiscard]]
		TOML(Int value);

		/// @brief 浮動小数点数型の値を作成します。
		/// @param value 値
		template <std::floating_point Float>
		[[nodiscard]]
		TOML(Float value);

		/// @brief 文字列型の値を作成します。
		/// @param s 文字列（UTF-8）
		[[nodiscard]]
		TOML(const char* s);

		/// @brief 文字列型の値を作成します。
		/// @param s 文字列（UTF-8）
		[[nodiscard]]
		TOML(std::string_view s);

		/// @brief 文字列型の値を作成します。
		/// @param s 文字列（UTF-8）
		[[nodiscard]]
		TOML(std::string s);

		/// @brief 文字列型の値を作成します。
		/// @param s 文字列
		[[nodiscard]]
		TOML(const char32* s);

		/// @brief 文字列型の値を作成します。
		/// @param s 文字列
		[[nodiscard]]
		TOML(StringView s);

		/// @brief 文字列型の値を作成します。
		/// @param s 文字列
		[[nodiscard]]
		TOML(const String& s);

		/// @brief local date 型の値を作成します。
		/// @param date 日付
		[[nodiscard]]
		TOML(const Date& date);

		/// @brief local time 型の値を作成します。
		/// @param time 時刻
		[[nodiscard]]
		TOML(const LocalTime& time);

		/// @brief local date-time 型の値を作成します。
		/// @param dateTime 日時
		[[nodiscard]]
		TOML(const LocalDateTime& dateTime);

		/// @brief offset date-time 型の値を作成します。
		/// @param dateTime 日時
		[[nodiscard]]
		TOML(const OffsetDateTime& dateTime);

		/// @brief 配列型の値を作成します。
		/// @tparam Type 要素の型
		/// @param array 要素の配列
		/// @remark `TOML{ Array<TOML>{ 1, U"text" } }` のように、異なる型の要素を持つ配列も作成できます。
		template <class Type>
			requires std::constructible_from<TOML, const Type&>
		[[nodiscard]]
		TOML(const Array<Type>& array);

		/// @brief テーブル型の値を作成します。
		/// @tparam Type 値の型
		/// @param table キーと値のハッシュテーブル
		/// @remark テーブルの要素の順序は、ハッシュテーブルの反復順序になります。
		template <class Type, class Hash, class Eq, class Alloc>
			requires std::constructible_from<TOML, const Type&>
		[[nodiscard]]
		TOML(const HashMap<String, Type, Hash, Eq, Alloc>& table);

		/// @brief テーブル型の値を作成します。
		/// @param table キーと値の組のリスト。`TOML{ { U"name", U"Siv3D" }, { U"size", 2 } }` のように指定します。
		/// @throw Error キーが重複している場合
		/// @remark テーブルの要素はリストの順序で並びます。
		[[nodiscard]]
		TOML(std::initializer_list<std::pair<StringView, TOML>> table);

		TOML& operator =(const TOML&) = default;

		TOML& operator =(TOML&&) = default;

		////////////////////////////////////////////////////////////////
		//
		//	Parse
		//
		////////////////////////////////////////////////////////////////

		/// @brief TOML 文字列をパースします。
		/// @param content TOML 文字列（UTF-8）
		/// @param pathHint 入力名のヒント
		/// @return パースに成功した場合は TOML, それ以外の場合は無効な値
		[[nodiscard]]
		static TOML Parse(std::string content, FilePathView pathHint = U"");

		/// @brief TOML 文字列をパースします。
		/// @param content TOML 文字列
		/// @param pathHint 入力名のヒント
		/// @return パースに成功した場合は TOML, それ以外の場合は無効な値
		[[nodiscard]]
		static TOML Parse(StringView content, FilePathView pathHint = U"");

		////////////////////////////////////////////////////////////////
		//
		//	ParseResult
		//
		////////////////////////////////////////////////////////////////

		/// @brief TOML 文字列をパースし、結果またはエラー理由を返します。
		/// @param content TOML 文字列（UTF-8）
		/// @param pathHint 入力名のヒント
		/// @return パースに成功した場合は TOML, それ以外の場合はエラー理由の配列
		[[nodiscard]]
		static Result<TOML, Array<TOMLParseErrorReason>> ParseResult(std::string content, FilePathView pathHint = U"");

		/// @brief TOML 文字列をパースし、結果またはエラー理由を返します。
		/// @param content TOML 文字列
		/// @param pathHint 入力名のヒント
		/// @return パースに成功した場合は TOML, それ以外の場合はエラー理由の配列
		[[nodiscard]]
		static Result<TOML, Array<TOMLParseErrorReason>> ParseResult(StringView content, FilePathView pathHint = U"");

		////////////////////////////////////////////////////////////////
		//
		//	Load
		//
		////////////////////////////////////////////////////////////////

		/// @brief UTF-8 の TOML ファイルをロードします。
		/// @param path ファイルパス
		/// @return ロードとパースに成功した場合は TOML, それ以外の場合は無効な値
		[[nodiscard]]
		static TOML Load(FilePathView path);

		/// @brief Reader から UTF-8 の TOML をロードします。
		/// @param reader Reader
		/// @return ロードとパースに成功した場合は TOML, それ以外の場合は無効な値
		[[nodiscard]]
		static TOML Load(std::unique_ptr<IReader> reader);

		/// @brief Reader から UTF-8 の TOML をロードします。
		/// @tparam Reader Reader オブジェクトの型
		/// @param reader Reader
		/// @return ロードとパースに成功した場合は TOML, それ以外の場合は無効な値
		template <ReaderObject Reader>
		[[nodiscard]]
		static TOML Load(Reader&& reader);

		////////////////////////////////////////////////////////////////
		//
		//	LoadResult
		//
		////////////////////////////////////////////////////////////////

		/// @brief UTF-8 の TOML ファイルをロードし、結果またはエラー理由を返します。
		/// @param path ファイルパス
		/// @return ロードとパースに成功した場合は TOML, それ以外の場合はエラー理由の配列
		[[nodiscard]]
		static Result<TOML, Array<TOMLParseErrorReason>> LoadResult(FilePathView path);

		/// @brief Reader から UTF-8 の TOML をロードし、結果またはエラー理由を返します。
		/// @param reader Reader
		/// @return ロードとパースに成功した場合は TOML, それ以外の場合はエラー理由の配列
		[[nodiscard]]
		static Result<TOML, Array<TOMLParseErrorReason>> LoadResult(std::unique_ptr<IReader> reader);

		/// @brief Reader から UTF-8 の TOML をロードし、結果またはエラー理由を返します。
		/// @tparam Reader Reader オブジェクトの型
		/// @param reader Reader
		/// @return ロードとパースに成功した場合は TOML, それ以外の場合はエラー理由の配列
		template <ReaderObject Reader>
		[[nodiscard]]
		static Result<TOML, Array<TOMLParseErrorReason>> LoadResult(Reader&& reader);

		////////////////////////////////////////////////////////////////
		//
		//	Invalid / EmptyArray / EmptyTable
		//
		////////////////////////////////////////////////////////////////

		/// @brief 無効な値を返します。
		/// @return 無効な値
		[[nodiscard]]
		static TOML Invalid();

		/// @brief 空の配列を返します。
		/// @return 空の配列
		[[nodiscard]]
		static TOML EmptyArray();

		/// @brief 空のテーブルを返します。
		/// @return 空のテーブル
		[[nodiscard]]
		static TOML EmptyTable();

	private:

		template <class>
		friend class detail::TOMLMixin;

		template <class>
		friend class detail::TOMLMutableMixin;

		friend TOMLRef;

		friend TOMLConstRef;

		[[nodiscard]]
		static TOML FromValue(Value value);

		[[nodiscard]]
		const Value& getValueImpl() const noexcept;

		[[nodiscard]]
		const Value* getRootImpl() const noexcept;

		[[nodiscard]]
		Value& getMutableValueImpl() noexcept;

		[[nodiscard]]
		Value* getMutableRootImpl() noexcept;

		Value m_value;
	};

	////////////////////////////////////////////////////////////////
	//
	//	TOMLRef
	//
	////////////////////////////////////////////////////////////////

	/// @brief TOML の値の要素を参照するクラス
	/// @remark 参照先を所有する TOML が破棄、ムーブ、または再代入されるか、参照先を含む配列またはテーブルに要素が追加または削除されると無効になります。
	class TOMLRef final : public detail::TOMLMutableMixin<TOMLRef>
	{
	public:

		[[nodiscard]]
		TOMLRef(const TOMLRef&) noexcept = default;

		/// @brief 参照先の値を、other の参照先の値の複製で置き換えます。
		/// @param other 参照
		/// @return *this
		/// @pre other が有効な参照であること。右辺は左辺より先に評価されるため、左辺の operator [] がキーを追加すると other は無効になり得ます。その場合は `TOML{ other }` を代入してください。
		TOMLRef& operator =(const TOMLRef& other);

		/// @brief 参照先の値を置き換えます。
		/// @param value 新しい値
		/// @return *this
		/// @pre value をムーブで作成する場合、ムーブ元は参照先を含んでいてはいけません。参照先を含む値を代入する場合は、ムーブせずにコピーしてください。
		TOMLRef& operator =(TOML value);

		/// @brief 2 つの参照先の値を交換します。
		/// @param lhs 一方の参照
		/// @param rhs もう一方の参照
		/// @pre 一方の参照先が、もう一方の参照先を含んでいてはいけません。
		friend void swap(TOMLRef lhs, TOMLRef rhs)
		{
			std::swap(*lhs.m_value, *rhs.m_value);
		}

	private:

		template <class>
		friend class detail::TOMLMixin;

		template <class>
		friend class detail::TOMLMutableMixin;

		template <bool>
		friend class TOMLArrayView;

		template <bool>
		friend class TOMLTableView;

		friend TOML;

		friend TOMLConstRef;

		[[nodiscard]]
		TOMLRef(detail::TOMLValue* root, detail::TOMLValue* value) noexcept;

		[[nodiscard]]
		const detail::TOMLValue& getValueImpl() const noexcept;

		[[nodiscard]]
		const detail::TOMLValue* getRootImpl() const noexcept;

		[[nodiscard]]
		detail::TOMLValue& getMutableValueImpl() noexcept;

		[[nodiscard]]
		detail::TOMLValue* getMutableRootImpl() noexcept;

		detail::TOMLValue* m_root;

		detail::TOMLValue* m_value;
	};

	////////////////////////////////////////////////////////////////
	//
	//	TOMLConstRef
	//
	////////////////////////////////////////////////////////////////

	/// @brief TOML の値または要素を読み取り専用で参照するクラス
	/// @remark 参照先を所有する TOML が破棄、ムーブ、または再代入されるか、参照先を含む配列またはテーブルに要素が追加または削除されると無効になります。
	class TOMLConstRef final : public detail::TOMLMixin<TOMLConstRef>
	{
	public:

		[[nodiscard]]
		TOMLConstRef(const TOMLConstRef&) noexcept = default;

		/// @brief TOML の値を参照します。
		/// @param value 参照する値
		[[nodiscard]]
		TOMLConstRef(const TOML& value) noexcept;

		/// @brief 参照先の値を読み取り専用で参照します。
		/// @param value 参照
		[[nodiscard]]
		TOMLConstRef(const TOMLRef& value) noexcept;

		TOMLConstRef& operator =(const TOMLConstRef&) = delete;

	private:

		template <class>
		friend class detail::TOMLMixin;

		template <bool>
		friend class TOMLArrayView;

		template <bool>
		friend class TOMLTableView;

		friend TOML;

		friend bool operator ==(TOMLConstRef lhs, TOMLConstRef rhs);

		[[nodiscard]]
		TOMLConstRef(const detail::TOMLValue* root, const detail::TOMLValue* value) noexcept;

		[[nodiscard]]
		const detail::TOMLValue& getValueImpl() const noexcept;

		[[nodiscard]]
		const detail::TOMLValue* getRootImpl() const noexcept;

		const detail::TOMLValue* m_root;

		const detail::TOMLValue* m_value;
	};

	/// @brief 2 つの値が等しいかを返します。
	/// @param lhs 一方の値
	/// @param rhs もう一方の値
	/// @return 2 つの値が等しい場合 true, それ以外の場合は false
	[[nodiscard]]
	bool operator ==(TOMLConstRef lhs, TOMLConstRef rhs);

	////////////////////////////////////////////////////////////////
	//
	//	TOMLTableMember
	//
	////////////////////////////////////////////////////////////////

	/// @brief TOML テーブルのメンバ
	/// @tparam Const 値を読み取り専用で参照する場合 true
	template <bool Const>
	struct TOMLTableMember
	{
		/// @brief キー
		String name;

		/// @brief 値への参照
		std::conditional_t<Const, TOMLConstRef, TOMLRef> value;
	};

	////////////////////////////////////////////////////////////////
	//
	//	TOMLArrayView
	//
	////////////////////////////////////////////////////////////////

	/// @brief TOML 配列の要素を参照するビュー
	/// @tparam Const 要素を読み取り専用で参照する場合 true
	template <bool Const>
	class TOMLArrayView
	{
	private:

		using RootPointer = std::conditional_t<Const, const detail::TOMLValue*, detail::TOMLValue*>;

		using ArrayType = std::conditional_t<Const, const detail::TOMLValue::array_type, detail::TOMLValue::array_type>;

		using ArrayIterator = std::conditional_t<Const, detail::TOMLValue::array_type::const_iterator, detail::TOMLValue::array_type::iterator>;

	public:

		/// @brief 要素への参照の型
		using reference = std::conditional_t<Const, TOMLConstRef, TOMLRef>;

		class Iterator
		{
		public:

			using difference_type = std::ptrdiff_t;
			using value_type = TOML;
			using iterator_category = std::forward_iterator_tag;

			[[nodiscard]]
			Iterator() = default;

			Iterator& operator ++();

			Iterator operator ++(int);

			[[nodiscard]]
			reference operator *() const;

			[[nodiscard]]
			friend bool operator ==(const Iterator& lhs, const Iterator& rhs) noexcept
			{
				return (lhs.m_it == rhs.m_it);
			}

		private:

			friend TOMLArrayView;

			[[nodiscard]]
			Iterator(RootPointer root, ArrayIterator it) noexcept;

			RootPointer m_root = nullptr;

			ArrayIterator m_it{};
		};

		[[nodiscard]]
		TOMLArrayView() = default;

		[[nodiscard]]
		TOMLArrayView(RootPointer root, ArrayType* array) noexcept;

		[[nodiscard]]
		Iterator begin() const;

		[[nodiscard]]
		Iterator end() const;

		[[nodiscard]]
		size_t size() const noexcept;

		[[nodiscard]]
		bool isEmpty() const noexcept;

	private:

		RootPointer m_root = nullptr;

		ArrayType* m_array = nullptr;
	};

	////////////////////////////////////////////////////////////////
	//
	//	TOMLTableView
	//
	////////////////////////////////////////////////////////////////

	/// @brief TOML テーブルのメンバを参照するビュー
	/// @tparam Const メンバの値を読み取り専用で参照する場合 true
	template <bool Const>
	class TOMLTableView
	{
	private:

		using RootPointer = std::conditional_t<Const, const detail::TOMLValue*, detail::TOMLValue*>;

		using TableType = std::conditional_t<Const, const detail::TOMLValue::table_type, detail::TOMLValue::table_type>;

		using TableIterator = std::conditional_t<Const, detail::TOMLValue::table_type::const_iterator, detail::TOMLValue::table_type::iterator>;

	public:

		class Iterator
		{
		public:

			using difference_type = std::ptrdiff_t;
			using value_type = TOMLTableMember<Const>;
			using iterator_category = std::forward_iterator_tag;

			[[nodiscard]]
			Iterator() = default;

			Iterator& operator ++();

			Iterator operator ++(int);

			[[nodiscard]]
			TOMLTableMember<Const> operator *() const;

			[[nodiscard]]
			friend bool operator ==(const Iterator& lhs, const Iterator& rhs) noexcept
			{
				return (lhs.m_it == rhs.m_it);
			}

		private:

			friend TOMLTableView;

			[[nodiscard]]
			Iterator(RootPointer root, TableIterator it) noexcept;

			RootPointer m_root = nullptr;

			TableIterator m_it{};
		};

		[[nodiscard]]
		TOMLTableView() = default;

		[[nodiscard]]
		TOMLTableView(RootPointer root, TableType* table) noexcept;

		[[nodiscard]]
		Iterator begin() const;

		[[nodiscard]]
		Iterator end() const;

		[[nodiscard]]
		size_t size() const noexcept;

		[[nodiscard]]
		bool isEmpty() const noexcept;

	private:

		RootPointer m_root = nullptr;

		TableType* m_table = nullptr;
	};
}

# include "detail/TOML.ipp"
