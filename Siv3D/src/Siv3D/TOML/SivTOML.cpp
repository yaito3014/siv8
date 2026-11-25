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

# include <Siv3D/TOML.hpp>
# include <Siv3D/Demangle.hpp>
# include <Siv3D/Error.hpp>
# include <Siv3D/FormatLiteral.hpp>
# include <Siv3D/TextEncoding.hpp>
# include <Siv3D/BinaryFileReader.hpp>

namespace s3d
{
	namespace
	{
		using detail::TOMLValue;

		[[nodiscard]]
		static String MakeShortValue(const String& s)
		{
			constexpr size_t MaxLength = 64;
			const bool truncated = (MaxLength < s.size());
			String result;

			for (const auto ch : s.substr(0, MaxLength))
			{
				switch (ch)
				{
				case U'\n': result.append(U"\\n"); break;
				case U'\r': result.append(U"\\r"); break;
				case U'\t': result.append(U"\\t"); break;
				default: result.push_back(ch); break;
				}
			}

			if (truncated)
			{
				result.append(U" ...");
			}

			return result;
		}

		[[nodiscard]]
		static int32 ToOffsetMinutes(const toml::time_offset& offset) noexcept
		{
			return ((static_cast<int32>(offset.hour) * 60) + static_cast<int32>(offset.minute));
		}

		[[nodiscard]]
		static TOMLValueType ToValueType(const TOMLValue& value) noexcept
		{
			switch (value.type())
			{
			case toml::value_t::boolean:         return TOMLValueType::Bool;
			case toml::value_t::integer:         return TOMLValueType::Int;
			case toml::value_t::floating:        return TOMLValueType::Float;
			case toml::value_t::string:          return TOMLValueType::String;
			case toml::value_t::offset_datetime: return TOMLValueType::OffsetDateTime;
			case toml::value_t::local_datetime:  return TOMLValueType::LocalDateTime;
			case toml::value_t::local_date:      return TOMLValueType::LocalDate;
			case toml::value_t::local_time:      return TOMLValueType::LocalTime;
			case toml::value_t::array:           return TOMLValueType::Array;
			case toml::value_t::table:           return TOMLValueType::Table;
			default:                             return TOMLValueType::Invalid;
			}
		}

		[[nodiscard]]
		static bool AppendPath(const TOMLValue& node, const TOMLValue* target, String& path)
		{
			if (&node == target)
			{
				return true;
			}

			const size_t length = path.size();

			if (node.is_table())
			{
				for (const auto& [key, value] : node.as_table())
				{
					path.append(U"[\"").append(Unicode::FromUTF8(key)).append(U"\"]");

					if (AppendPath(value, target, path))
					{
						return true;
					}

					path.resize(length);
				}
			}
			else if (node.is_array())
			{
				const auto& array = node.as_array();

				for (size_t i = 0; i < array.size(); ++i)
				{
					path.append(U"[{}]"_fmt(i));

					if (AppendPath(array[i], target, path))
					{
						return true;
					}

					path.resize(length);
				}
			}

			return false;
		}

		[[nodiscard]]
		static String MakePath(const TOMLValue* root, const TOMLValue& value)
		{
			String path = U"$";

			if (root && (root != &value) && (not AppendPath(*root, &value, path)))
			{
				return U"$";
			}

			return path;
		}

		[[nodiscard]]
		static String FormatForMessage(const TOMLValue& value)
		{
			if (value.is_empty())
			{
				return U"";
			}

			try
			{
				return Unicode::FromUTF8(toml::format(value, toml::spec::default_version()));
			}
			catch (const toml::exception&)
			{
				return U"";
			}
		}

		[[nodiscard]]
		static std::string FormatValue(const TOMLValue& value)
		{
			if (value.is_empty())
			{
				return{};
			}

			try
			{
				return toml::format(value, toml::spec::default_version());
			}
			catch (const toml::exception& e)
			{
				throw Error{ U"TOML::format(): {}"_fmt(Unicode::FromUTF8(e.what())) };
			}
		}

		[[nodiscard]]
		static std::string MakeParseName(const FilePathView pathHint)
		{
			return pathHint ? Unicode::ToUTF8(pathHint) : std::string{ "internal string" };
		}

		[[nodiscard]]
		static Array<TOMLParseErrorReason> ToParseErrorReasons(const std::vector<toml::error_info>& internalErrors)
		{
			Array<TOMLParseErrorReason> errors;
			errors.reserve(internalErrors.size());

			for (const auto& internalError : internalErrors)
			{
				TOMLParseErrorReason error;
				error.title = Unicode::FromUTF8(internalError.title());
				error.hint = Unicode::FromUTF8(internalError.suffix());

				for (const auto& location : internalError.locations())
				{
					error.locations.push_back(Unicode::FromUTF8(toml::format_location(location.first, location.second)));
				}

				errors.push_back(std::move(error));
			}

			return errors;
		}

		[[nodiscard]]
		static bool ReadTextFile(IReader& reader, std::string& content)
		{
			content.clear();

			if (not reader.isOpen())
			{
				return false;
			}

			const TextEncoding encoding = Unicode::GetTextEncoding(reader);
			if ((encoding != TextEncoding::UTF8_NO_BOM) && (encoding != TextEncoding::UTF8_WITH_BOM))
			{
				return false;
			}

			const int64 start = Unicode::GetBOMSize(encoding);
			const int64 size = (reader.size() - start);
			if (size < 0)
			{
				return false;
			}

			content.resize(static_cast<size_t>(size));
			if (size == 0)
			{
				return true;
			}

			if (size != reader.read(content.data(), start, size))
			{
				content.clear();
				return false;
			}

			return true;
		}

		[[nodiscard]]
		static Result<TOMLValue, Array<TOMLParseErrorReason>> ParseValue(std::string content, const FilePathView pathHint)
		{
			std::istringstream iss{ std::move(content) };

			if (auto parseResult = toml::try_parse<toml::ordered_type_config>(iss, MakeParseName(pathHint), toml::spec::default_version()))
			{
				return std::move(parseResult.unwrap());
			}
			else
			{
				return Err{ ToParseErrorReasons(parseResult.as_err()) };
			}
		}

		[[noreturn]]
		static void ThrowNotTable(const StringView function, const std::string& key, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::{}(\"{}\"): value at {} is not a table. Actual type: {}"_fmt(function, Unicode::FromUTF8(key), MakePath(root, value), ToValueType(value)) };
		}

		[[noreturn]]
		static void ThrowKeyNotFound(const StringView function, const std::string& key, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::{}(\"{}\"): key was not found at {}"_fmt(function, Unicode::FromUTF8(key), MakePath(root, value)) };
		}

		[[noreturn]]
		static void ThrowNotArray(const StringView function, const size_t index, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::{}({}): value at {} is not an array. Actual type: {}"_fmt(function, index, MakePath(root, value), ToValueType(value)) };
		}

		[[noreturn]]
		static void ThrowIndexOutOfRange(const StringView function, const size_t index, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::{}({}): index out of range at {}. Size: {}"_fmt(function, index, MakePath(root, value), value.as_array().size()) };
		}

		[[noreturn]]
		static void ThrowWrongType(const StringView function, const StringView expected, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::{}(): value at {} is not {}. Actual type: {}"_fmt(function, MakePath(root, value), expected, ToValueType(value)) };
		}

		[[nodiscard]]
		static const TOMLValue& FindMember(const StringView function, const TOMLValue* root, const TOMLValue& value, const std::string& key)
		{
			if (not value.is_table())
			{
				ThrowNotTable(function, key, root, value);
			}

			const auto& table = value.as_table();
			const auto it = table.find(key);

			if (it == table.end())
			{
				ThrowKeyNotFound(function, key, root, value);
			}

			return it->second;
		}

		[[nodiscard]]
		static TOMLValue& FindMember(const StringView function, const TOMLValue* root, TOMLValue& value, const std::string& key)
		{
			return const_cast<TOMLValue&>(FindMember(function, root, std::as_const(value), key));
		}

		[[nodiscard]]
		static const TOMLValue& FindElement(const StringView function, const TOMLValue* root, const TOMLValue& value, const size_t index)
		{
			if (not value.is_array())
			{
				ThrowNotArray(function, index, root, value);
			}

			const auto& array = value.as_array();

			if (array.size() <= index)
			{
				ThrowIndexOutOfRange(function, index, root, value);
			}

			return array[index];
		}

		[[nodiscard]]
		static TOMLValue& FindElement(const StringView function, const TOMLValue* root, TOMLValue& value, const size_t index)
		{
			return const_cast<TOMLValue&>(FindElement(function, root, std::as_const(value), index));
		}
	}

	namespace detail
	{
		////////////////////////////////////////////////////////////////
		//
		//	TOMLLocalTime / TOMLLocalDateTime / TOMLOffsetDateTime
		//
		////////////////////////////////////////////////////////////////

		DateTime TOMLLocalTime::toDateTime() const noexcept
		{
			return DateTime{ 0, 0, 0, hour, minute, second, millisecond };
		}

		DateTime TOMLLocalTime::toDateTimeTruncatedToMilliseconds() const noexcept
		{
			return toDateTime();
		}

		TOMLLocalTime TOMLLocalTime::FromTOML(const toml::local_time& localTime) noexcept
		{
			return TOMLLocalTime{
				static_cast<uint8>(localTime.hour),
				static_cast<uint8>(localTime.minute),
				static_cast<uint8>(localTime.second),
				static_cast<uint16>(localTime.millisecond),
				static_cast<uint16>(localTime.microsecond),
				static_cast<uint16>(localTime.nanosecond)
			};
		}

		DateTime TOMLLocalDateTime::toDateTime() const noexcept
		{
			return DateTime{ date, time.hour, time.minute, time.second, time.millisecond };
		}

		DateTime TOMLLocalDateTime::toDateTimeTruncatedToMilliseconds() const noexcept
		{
			return toDateTime();
		}

		TOMLLocalDateTime TOMLLocalDateTime::FromTOML(const toml::local_datetime& localDateTime) noexcept
		{
			return TOMLLocalDateTime{ ToDate(localDateTime.date), TOMLLocalTime::FromTOML(localDateTime.time) };
		}

		TOMLOffsetDateTime TOMLOffsetDateTime::FromTOML(const toml::offset_datetime& offsetDateTime) noexcept
		{
			return TOMLOffsetDateTime{
				TOMLLocalDateTime{ ToDate(offsetDateTime.date), TOMLLocalTime::FromTOML(offsetDateTime.time) },
				ToOffsetMinutes(offsetDateTime.offset)
			};
		}

		Date ToDate(const toml::local_date& date) noexcept
		{
			return Date{ static_cast<int32>(date.year), (static_cast<int32>(date.month) + 1), static_cast<int32>(date.day) };
		}

		void ThrowTOMLGetError(const char* type, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::get<{}>() failed at {}. Actual type: {}, value: \"{}\""_fmt(Demangle(type), MakePath(root, value), ToValueType(value), MakeShortValue(FormatForMessage(value))) };
		}

		void ThrowTOMLParsedGetError(const char* type, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::getParsed<{}>() failed at {}. Actual type: {}, value: \"{}\""_fmt(Demangle(type), MakePath(root, value), ToValueType(value), MakeShortValue(FormatForMessage(value))) };
		}

		void ThrowTOMLNotArray(const char* type, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::getArray<{}>(): value at {} is not an array. Actual type: {}"_fmt(Demangle(type), MakePath(root, value), ToValueType(value)) };
		}

		void ThrowTOMLArrayElementGetError(const char* type, const TOMLValue* root, const TOMLValue& value)
		{
			throw Error{ U"TOML::getArray<{}>(): element conversion failed at {}. Actual type: {}, value: \"{}\""_fmt(Demangle(type), MakePath(root, value), ToValueType(value), MakeShortValue(FormatForMessage(value))) };
		}

		void ThrowTOMLIntegerOutOfRange(const uint64 value)
		{
			throw Error{ U"TOML::TOML(): integer {} is out of the range of int64"_fmt(value) };
		}

		////////////////////////////////////////////////////////////////
		//
		//	TOMLMixin
		//
		////////////////////////////////////////////////////////////////

		template <class Derived>
		bool TOMLMixin<Derived>::isValid() const noexcept
		{
			return (not getValue().is_empty());
		}

		template <class Derived>
		TOMLMixin<Derived>::operator bool() const noexcept
		{
			return isValid();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isInvalid() const noexcept
		{
			return getValue().is_empty();
		}

		template <class Derived>
		String TOMLMixin<Derived>::path() const
		{
			return MakePath(getRoot(), getValue());
		}

		template <class Derived>
		TOMLValueType TOMLMixin<Derived>::type() const noexcept
		{
			return ToValueType(getValue());
		}

		template <class Derived>
		TOMLValueType TOMLMixin<Derived>::getType() const noexcept
		{
			return type();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isBool() const noexcept
		{
			return getValue().is_boolean();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isInt() const noexcept
		{
			return getValue().is_integer();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isFloat() const noexcept
		{
			return getValue().is_floating();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isString() const noexcept
		{
			return getValue().is_string();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isLocalDate() const noexcept
		{
			return getValue().is_local_date();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isLocalTime() const noexcept
		{
			return getValue().is_local_time();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isLocalDateTime() const noexcept
		{
			return getValue().is_local_datetime();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isOffsetDateTime() const noexcept
		{
			return getValue().is_offset_datetime();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isArray() const noexcept
		{
			return getValue().is_array();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isTable() const noexcept
		{
			return getValue().is_table();
		}

		template <class Derived>
		bool TOMLMixin<Derived>::is(const TOMLValueType valueType) const noexcept
		{
			return (type() == valueType);
		}

		template <class Derived>
		bool TOMLMixin<Derived>::contains(const std::string& key) const
		{
			const TOMLValue& value = getValue();
			return (value.is_table() && value.as_table().contains(key));
		}

		template <class Derived>
		bool TOMLMixin<Derived>::contains(const StringView key) const
		{
			return contains(Unicode::ToUTF8(key));
		}

		template <class Derived>
		size_t TOMLMixin<Derived>::size() const noexcept
		{
			const TOMLValue& value = getValue();

			if (value.is_array())
			{
				return value.as_array().size();
			}

			if (value.is_table())
			{
				return value.as_table().size();
			}

			return 0;
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isEmptyArray() const noexcept
		{
			const TOMLValue& value = getValue();
			return (value.is_array() && value.as_array().empty());
		}

		template <class Derived>
		bool TOMLMixin<Derived>::isEmptyTable() const noexcept
		{
			const TOMLValue& value = getValue();
			return (value.is_table() && value.as_table().empty());
		}

		template <class Derived>
		TOMLConstRef TOMLMixin<Derived>::operator [](const std::string& key) const
		{
			return TOMLConstRef{ getRoot(), &FindMember(U"operator[]", getRoot(), getValue(), key) };
		}

		template <class Derived>
		TOMLConstRef TOMLMixin<Derived>::operator [](const StringView key) const
		{
			return operator [](Unicode::ToUTF8(key));
		}

		template <class Derived>
		TOMLConstRef TOMLMixin<Derived>::operator [](const size_t index) const
		{
			return TOMLConstRef{ getRoot(), &FindElement(U"operator[]", getRoot(), getValue(), index) };
		}

		template <class Derived>
		TOMLConstRef TOMLMixin<Derived>::at(const std::string& key) const
		{
			return TOMLConstRef{ getRoot(), &FindMember(U"at", getRoot(), getValue(), key) };
		}

		template <class Derived>
		TOMLConstRef TOMLMixin<Derived>::at(const StringView key) const
		{
			return at(Unicode::ToUTF8(key));
		}

		template <class Derived>
		TOMLConstRef TOMLMixin<Derived>::at(const size_t index) const
		{
			return TOMLConstRef{ getRoot(), &FindElement(U"at", getRoot(), getValue(), index) };
		}

		template <class Derived>
		String TOMLMixin<Derived>::getString() const
		{
			return get<String>();
		}

		template <class Derived>
		std::string TOMLMixin<Derived>::getUTF8() const
		{
			return get<std::string>();
		}

		template <class Derived>
		TOMLLocalTime TOMLMixin<Derived>::getLocalTime() const
		{
			return get<TOMLLocalTime>();
		}

		template <class Derived>
		Date TOMLMixin<Derived>::getLocalDate() const
		{
			return get<Date>();
		}

		template <class Derived>
		TOMLLocalDateTime TOMLMixin<Derived>::getLocalDateTime() const
		{
			return get<TOMLLocalDateTime>();
		}

		template <class Derived>
		TOMLOffsetDateTime TOMLMixin<Derived>::getOffsetDateTime() const
		{
			return get<TOMLOffsetDateTime>();
		}

		template <class Derived>
		TOMLArrayView<true> TOMLMixin<Derived>::arrayView() const
		{
			const TOMLValue& value = getValue();

			if (not value.is_array())
			{
				ThrowWrongType(U"arrayView", U"an array", getRoot(), value);
			}

			return TOMLArrayView<true>{ getRoot(), &value.as_array() };
		}

		template <class Derived>
		TOMLTableView<true> TOMLMixin<Derived>::tableView() const
		{
			const TOMLValue& value = getValue();

			if (not value.is_table())
			{
				ThrowWrongType(U"tableView", U"a table", getRoot(), value);
			}

			return TOMLTableView<true>{ getRoot(), &value.as_table() };
		}

		template <class Derived>
		String TOMLMixin<Derived>::format() const
		{
			return Unicode::FromUTF8(FormatValue(getValue()));
		}

		template <class Derived>
		std::string TOMLMixin<Derived>::formatUTF8() const
		{
			return FormatValue(getValue());
		}

		////////////////////////////////////////////////////////////////
		//
		//	TOMLMutableMixin
		//
		////////////////////////////////////////////////////////////////

		template <class Derived>
		TOMLRef TOMLMutableMixin<Derived>::operator [](const std::string& key)
		{
			TOMLValue& value = getMutableValue();

			if (value.is_empty())
			{
				value = TOMLValue{ TOMLValue::table_type{} };
			}
			else if (not value.is_table())
			{
				ThrowNotTable(U"operator[]", key, getMutableRoot(), value);
			}

			return TOMLRef{ getMutableRoot(), &value.as_table()[key] };
		}

		template <class Derived>
		TOMLRef TOMLMutableMixin<Derived>::operator [](const StringView key)
		{
			return operator [](Unicode::ToUTF8(key));
		}

		template <class Derived>
		TOMLRef TOMLMutableMixin<Derived>::operator [](const size_t index)
		{
			return TOMLRef{ getMutableRoot(), &FindElement(U"operator[]", getMutableRoot(), getMutableValue(), index) };
		}

		template <class Derived>
		TOMLRef TOMLMutableMixin<Derived>::at(const std::string& key)
		{
			return TOMLRef{ getMutableRoot(), &FindMember(U"at", getMutableRoot(), getMutableValue(), key) };
		}

		template <class Derived>
		TOMLRef TOMLMutableMixin<Derived>::at(const StringView key)
		{
			return at(Unicode::ToUTF8(key));
		}

		template <class Derived>
		TOMLRef TOMLMutableMixin<Derived>::at(const size_t index)
		{
			return TOMLRef{ getMutableRoot(), &FindElement(U"at", getMutableRoot(), getMutableValue(), index) };
		}

		template <class Derived>
		void TOMLMutableMixin<Derived>::push_back(TOML value)
		{
			TOMLValue& self = getMutableValue();

			if (self.is_empty())
			{
				self = TOMLValue{ TOMLValue::array_type{} };
			}
			else if (not self.is_array())
			{
				ThrowWrongType(U"push_back", U"an array", getMutableRoot(), self);
			}

			self.as_array().push_back(std::move(value.m_value));
		}

		template <class Derived>
		void TOMLMutableMixin<Derived>::pop_back()
		{
			TOMLValue& value = getMutableValue();

			if (not value.is_array())
			{
				ThrowWrongType(U"pop_back", U"an array", getMutableRoot(), value);
			}

			auto& array = value.as_array();

			if (array.empty())
			{
				throw Error{ U"TOML::pop_back(): array at {} is empty"_fmt(MakePath(getMutableRoot(), value)) };
			}

			array.pop_back();
		}

		template <class Derived>
		void TOMLMutableMixin<Derived>::clear()
		{
			TOMLValue& value = getMutableValue();

			if (value.is_array())
			{
				value.as_array().clear();
			}
			else if (value.is_table())
			{
				value.as_table().clear();
			}
			else
			{
				ThrowWrongType(U"clear", U"an array or a table", getMutableRoot(), value);
			}
		}

		template <class Derived>
		void TOMLMutableMixin<Derived>::erase(const std::string& key)
		{
			TOMLValue& value = getMutableValue();

			if (not value.is_table())
			{
				ThrowNotTable(U"erase", key, getMutableRoot(), value);
			}

			value.as_table().erase(key);
		}

		template <class Derived>
		void TOMLMutableMixin<Derived>::erase(const StringView key)
		{
			erase(Unicode::ToUTF8(key));
		}

		template <class Derived>
		void TOMLMutableMixin<Derived>::erase(const size_t index)
		{
			TOMLValue& value = getMutableValue();

			if (not value.is_array())
			{
				ThrowNotArray(U"erase", index, getMutableRoot(), value);
			}

			auto& array = value.as_array();

			if (array.size() <= index)
			{
				ThrowIndexOutOfRange(U"erase", index, getMutableRoot(), value);
			}

			array.erase(array.begin() + index);
		}

		template <class Derived>
		TOMLArrayView<false> TOMLMutableMixin<Derived>::arrayView()
		{
			TOMLValue& value = getMutableValue();

			if (not value.is_array())
			{
				ThrowWrongType(U"arrayView", U"an array", getMutableRoot(), value);
			}

			return TOMLArrayView<false>{ getMutableRoot(), &value.as_array() };
		}

		template <class Derived>
		TOMLTableView<false> TOMLMutableMixin<Derived>::tableView()
		{
			TOMLValue& value = getMutableValue();

			if (not value.is_table())
			{
				ThrowWrongType(U"tableView", U"a table", getMutableRoot(), value);
			}

			return TOMLTableView<false>{ getMutableRoot(), &value.as_table() };
		}

		template class TOMLMixin<TOML>;
		template class TOMLMixin<TOMLRef>;
		template class TOMLMixin<TOMLConstRef>;
		template class TOMLMutableMixin<TOML>;
		template class TOMLMutableMixin<TOMLRef>;
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOML
	//
	////////////////////////////////////////////////////////////////

	TOML::TOML(const TOMLConstRef value)
		: m_value(value.getValueImpl()) {}

	TOML::TOML(const TOMLRef value)
		: m_value(value.getValueImpl()) {}

	TOML::TOML(const char* s)
		: m_value(std::string{ s }) {}

	TOML::TOML(const std::string_view s)
		: m_value(std::string{ s }) {}

	TOML::TOML(std::string s)
		: m_value(std::move(s)) {}

	TOML::TOML(const char32* s)
		: m_value(Unicode::ToUTF8(StringView{ s })) {}

	TOML::TOML(const StringView s)
		: m_value(Unicode::ToUTF8(s)) {}

	TOML::TOML(const String& s)
		: m_value(Unicode::ToUTF8(s)) {}

	TOML::TOML(const Date& date)
		: m_value(toml::local_date{ date.year, static_cast<toml::month_t>(date.month - 1), date.day }) {}

	TOML::TOML(const LocalTime& time)
		: m_value(toml::local_time{ time.hour, time.minute, time.second, time.millisecond, time.microsecond, time.nanosecond }) {}

	TOML::TOML(const LocalDateTime& dateTime)
		: m_value(toml::local_datetime{
			toml::local_date{ dateTime.date.year, static_cast<toml::month_t>(dateTime.date.month - 1), dateTime.date.day },
			toml::local_time{ dateTime.time.hour, dateTime.time.minute, dateTime.time.second, dateTime.time.millisecond, dateTime.time.microsecond, dateTime.time.nanosecond } }) {}

	TOML::TOML(const OffsetDateTime& dateTime)
		: m_value(toml::offset_datetime{
			toml::local_datetime{
				toml::local_date{ dateTime.localDateTime.date.year, static_cast<toml::month_t>(dateTime.localDateTime.date.month - 1), dateTime.localDateTime.date.day },
				toml::local_time{ dateTime.localDateTime.time.hour, dateTime.localDateTime.time.minute, dateTime.localDateTime.time.second,
					dateTime.localDateTime.time.millisecond, dateTime.localDateTime.time.microsecond, dateTime.localDateTime.time.nanosecond } },
			toml::time_offset{ (dateTime.offsetMinutes / 60), (dateTime.offsetMinutes % 60) } }) {}

	TOML::TOML(const std::initializer_list<std::pair<StringView, TOML>> table)
		: m_value(Value::table_type{})
	{
		auto& members = m_value.as_table();

		for (const auto& [key, value] : table)
		{
			std::string key8 = Unicode::ToUTF8(key);

			if (members.contains(key8))
			{
				throw Error{ U"TOML::TOML(): duplicate key \"{}\""_fmt(key) };
			}

			members.emplace(std::move(key8), value.m_value);
		}
	}

	TOML TOML::Parse(std::string content, const FilePathView pathHint)
	{
		if (auto result = ParseValue(std::move(content), pathHint))
		{
			return FromValue(std::move(*result));
		}

		return{};
	}

	TOML TOML::Parse(const StringView content, const FilePathView pathHint)
	{
		return Parse(Unicode::ToUTF8(content), pathHint);
	}

	Result<TOML, Array<TOMLParseErrorReason>> TOML::ParseResult(std::string content, const FilePathView pathHint)
	{
		if (auto result = ParseValue(std::move(content), pathHint))
		{
			return FromValue(std::move(*result));
		}
		else
		{
			return Err{ std::move(result.error()) };
		}
	}

	Result<TOML, Array<TOMLParseErrorReason>> TOML::ParseResult(const StringView content, const FilePathView pathHint)
	{
		return ParseResult(Unicode::ToUTF8(content), pathHint);
	}

	TOML TOML::Load(const FilePathView path)
	{
		std::string content;
		{
			BinaryFileReader reader{ path };

			if (not ReadTextFile(reader, content))
			{
				return{};
			}
		}

		return Parse(std::move(content), path);
	}

	TOML TOML::Load(std::unique_ptr<IReader> reader)
	{
		if (not reader)
		{
			return{};
		}

		std::string content;
		{
			if (not ReadTextFile(*reader, content))
			{
				return{};
			}

			reader.reset();
		}

		return Parse(std::move(content));
	}

	Result<TOML, Array<TOMLParseErrorReason>> TOML::LoadResult(const FilePathView path)
	{
		std::string content;
		{
			BinaryFileReader reader{ path };

			if (not ReadTextFile(reader, content))
			{
				return Err{ Array<TOMLParseErrorReason>{ TOMLParseErrorReason{ U"Failed to read the file.", { String{ path } }, U"Check if the file exists and is a valid UTF-8 text file." } } };
			}
		}

		return ParseResult(std::move(content), path);
	}

	Result<TOML, Array<TOMLParseErrorReason>> TOML::LoadResult(std::unique_ptr<IReader> reader)
	{
		if (not reader)
		{
			return Err{ Array<TOMLParseErrorReason>{ TOMLParseErrorReason{ U"Reader is null.", {}, U"Provide a valid reader." } } };
		}

		std::string content;
		{
			if (not ReadTextFile(*reader, content))
			{
				return Err{ Array<TOMLParseErrorReason>{ TOMLParseErrorReason{ U"Failed to read the file.", {}, U"Check if the file is a valid UTF-8 text file." } } };
			}

			reader.reset();
		}

		return ParseResult(std::move(content));
	}

	TOML TOML::Invalid()
	{
		return{};
	}

	TOML TOML::EmptyArray()
	{
		return FromValue(Value::array_type{});
	}

	TOML TOML::EmptyTable()
	{
		return FromValue(Value::table_type{});
	}

	TOML TOML::FromValue(Value value)
	{
		TOML result;
		result.m_value = std::move(value);
		return result;
	}

	////////////////////////////////////////////////////////////////
	//
	//	TOMLRef
	//
	////////////////////////////////////////////////////////////////

	TOMLRef& TOMLRef::operator =(const TOMLRef& other)
	{
		if (m_value != other.m_value)
		{
			TOMLValue copy = *other.m_value;
			*m_value = std::move(copy);
		}

		return *this;
	}

	TOMLRef& TOMLRef::operator =(TOML value)
	{
		*m_value = std::move(value.m_value);
		return *this;
	}

	bool operator ==(const TOMLConstRef lhs, const TOMLConstRef rhs)
	{
		return (lhs.getValueImpl() == rhs.getValueImpl());
	}
}
