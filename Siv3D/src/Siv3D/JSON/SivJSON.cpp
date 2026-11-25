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

# include <algorithm>
# include <Siv3D/JSON.hpp>
# include <Siv3D/JSONIterator.hpp>
# include <Siv3D/FormatLiteral.hpp>
# include <Siv3D/Unicode.hpp>
# include <Siv3D/Demangle.hpp>
# include <Siv3D/TextFileWriter.hpp>
# include <Siv3D/BinaryFileReader.hpp>

namespace s3d
{
	namespace
	{
		using JSONErrors = Array<JSONParseErrorReason>;

		[[nodiscard]]
		static Err<JSONErrors> MakeJSONError(String title, Array<String> locations = {}, String hint = {})
		{
			return Err{ JSONErrors{ JSONParseErrorReason{ std::move(title), std::move(locations), std::move(hint) } } };
		}

		[[nodiscard]]
		static String MakeParseName(const FilePathView pathHint)
		{
			return pathHint ? String{ pathHint } : String{ U"internal string" };
		}

		// byte is the 1-based count of bytes read up to and including the offending one, as reported by nlohmann.
		[[nodiscard]]
		static String MakeLocation(const FilePathView pathHint, const std::string_view content, const std::size_t byte)
		{
			const std::size_t index = std::min(((byte == 0) ? 0 : (byte - 1)), content.size());
			const std::string_view before = content.substr(0, index);
			const std::size_t line = (1 + static_cast<std::size_t>(std::ranges::count(before, '\n')));
			const std::size_t lineBegin = ((before.rfind('\n') == std::string_view::npos) ? 0 : (before.rfind('\n') + 1));
			return U"{}:{}:{}"_fmt(MakeParseName(pathHint), line, (index - lineBegin + 1));
		}

		[[nodiscard]]
		static String MakeTitle(const detail::JSONBase::exception& e)
		{
			std::string_view message = e.what();

			if (const std::size_t pos = message.find("] "); pos != std::string_view::npos)
			{
				message.remove_prefix(pos + 2);
			}

			if (message.starts_with("parse error"))
			{
				if (const std::size_t pos = message.find(": "); pos != std::string_view::npos)
				{
					message.remove_prefix(pos + 2);
				}
			}

			// The message quotes the input as read, which may be invalid UTF-8; FromUTF8 would return an empty string for it.
			String title;

			while (not message.empty())
			{
				const auto valid = Unicode::ValidateUTF8(message);
				const std::size_t length = (valid ? message.size() : valid.error());
				title.append(Unicode::FromUTF8(message.substr(0, length)));
				message.remove_prefix(length);

				if (not message.empty())
				{
					title.push_back(U'�');
					message.remove_prefix(1);
				}
			}

			return title;
		}

		[[nodiscard]]
		static Result<detail::JSONBase, JSONErrors> ParseJSON(const std::string_view content, const FilePathView pathHint)
		{
			if (const std::size_t pos = content.find('\0'); pos != std::string_view::npos)
			{
				return MakeJSONError(U"unescaped NUL in input", { MakeLocation(pathHint, content, (pos + 1)) });
			}

			try
			{
				return detail::JSONBase::parse(content, nullptr, true, true);
			}
			catch (const detail::JSONBase::parse_error& e)
			{
				return MakeJSONError(MakeTitle(e), { MakeLocation(pathHint, content, e.byte) });
			}
			catch (const detail::JSONBase::exception& e)
			{
				return MakeJSONError(MakeTitle(e));
			}
		}

		// Reads the remaining bytes as UTF-8, converting UTF-16 with a BOM.
		[[nodiscard]]
		static Result<std::string, JSONErrors> ReadJSONText(IReader& reader, const FilePathView pathHint)
		{
			const Array<String> locations = (pathHint ? Array<String>{ String{ pathHint } } : Array<String>{});

			if (not reader.isOpen())
			{
				return MakeJSONError(U"Failed to open the input.", locations, U"Check if the file exists.");
			}

			const int64 size = (reader.size() - reader.getPos());

			if (size < 0)
			{
				return MakeJSONError(U"Invalid reader position.", locations);
			}

			std::string bytes(static_cast<std::size_t>(size), '\0');
			int64 offset = 0;

			while (offset < size)
			{
				const int64 count = reader.read((bytes.data() + offset), (size - offset));

				if ((count <= 0) || ((size - offset) < count))
				{
					return MakeJSONError(U"Failed to read the input.", locations);
				}

				offset += count;
			}

			// Preserve CR and NUL: text-reader normalization can change JSON tokens.
			const bool littleEndian = bytes.starts_with("\xFF\xFE");

			if (littleEndian || bytes.starts_with("\xFE\xFF"))
			{
				if (bytes.size() % 2)
				{
					return MakeJSONError(U"Incomplete UTF-16 code unit.", locations);
				}

				std::u16string utf16(((bytes.size() / 2) - 1), u'\0');

				for (std::size_t i = 0; i < utf16.size(); ++i)
				{
					const uint16 first = static_cast<uint8>(bytes[2 + i * 2]);
					const uint16 second = static_cast<uint8>(bytes[3 + i * 2]);
					utf16[i] = static_cast<char16>(littleEndian ? (first | (second << 8)) : ((first << 8) | second));
				}

				return Unicode::UTF16ToUTF8(utf16);
			}

			return bytes;
		}

		[[nodiscard]]
		static Result<JSON, JSONErrors> LoadJSON(IReader& reader, const FilePathView pathHint)
		{
			auto text = ReadJSONText(reader, pathHint);

			if (not text)
			{
				return Err{ std::move(text).error() };
			}

			return JSON::ParseResult(std::move(*text), pathHint);
		}

		template <class JSONBaseType>
		[[nodiscard]]
		static JSONBaseType& CheckedAt(JSONBaseType& json, const std::size_t index)
		{
			if (not json.is_array())
			{
				throw Error{ "JSON::operator [](size_t): This JSON value is not an array" };
			}

			if (json.size() <= index)
			{
				throw Error{ "JSON::operator [](size_t): Index out of range" };
			}

			return json[index];
		}

		static void PushBack(detail::JSONBase& json, detail::JSONBase&& value)
		{
			if (not (json.is_null() || json.is_array()))
			{
				throw Error{ "JSON::push_back(): This JSON value is not an array" };
			}

			json.push_back(std::move(value));
		}

		static void EraseIndex(detail::JSONBase& json, const std::size_t index)
		{
			if (not json.is_array())
			{
				throw Error{ "JSON::erase(size_t): This JSON value is not an array" };
			}

			if (json.size() <= index)
			{
				throw Error{ "JSON::erase(size_t): Index out of range" };
			}

			json.erase(json.begin() + index);
		}

		static void PopBack(detail::JSONBase& json)
		{
			if (not json.is_array())
			{
				throw Error{ "JSON::pop_back(): This JSON value is not an array" };
			}

			if (json.empty())
			{
				throw Error{ "JSON::pop_back(): The array is empty" };
			}

			json.erase(json.end() - 1);
		}

		static void EraseKey(detail::JSONBase& json, const std::string_view key)
		{
			if (not json.is_object())
			{
				throw Error{ "JSON::erase(): This JSON value is not an object" };
			}

			json.erase(std::string{ key });
		}

		[[nodiscard]]
		static Blob ToBlob(const std::vector<uint8>& data)
		{
			return Blob{ data.data(), data.size() };
		}

		[[nodiscard]]
		static bool SaveText(const FilePathView path, const std::string& text)
		{
			TextFileWriter writer{ path };

			if (not writer)
			{
				return false;
			}

			writer.writeUTF8(text);
			return true;
		}

		template <class Function>
		[[nodiscard]]
		static JSON FromBinary(const char* name, const AllowExceptions allowExceptions, Function f)
		{
			try
			{
				return JSON(f(allowExceptions.getBool()));
			}
			catch (const std::exception& e)
			{
				throw Error{ fmt::format("JSON::{}(): {}", name, e.what()) };
			}
		}
	}

	namespace detail
	{
		void ThrowJSONGetError(const char* type, const std::string_view s)
		{
			if (s.size() <= 64)
			{
				throw Error{ fmt::format("JSON::get<{}>(\"{}\") failed", DemangleUTF8(type), s) };
			}
			else
			{
				throw Error{ fmt::format("JSON::get<{}>(\"{}\"...) failed", DemangleUTF8(type), s.substr(0, 64)) };
			}
		}

		template <class Derived>
		JSONValueType JSONMixin<Derived>::getType() const noexcept
		{
			return static_cast<JSONValueType>(getRef().type());
		}

		template <class Derived>
		bool JSONMixin<Derived>::isNull() const noexcept
		{
			return getRef().is_null();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isObject() const noexcept
		{
			return getRef().is_object();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isArray() const noexcept
		{
			return getRef().is_array();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isString() const noexcept
		{
			return getRef().is_string();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isPrimitive() const noexcept
		{
			return getRef().is_primitive();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isBool() const noexcept
		{
			return getRef().is_boolean();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isNumber() const noexcept
		{
			return getRef().is_number();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isInt() const noexcept
		{
			return getRef().is_number_integer();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isUint() const noexcept
		{
			return getRef().is_number_unsigned();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isFloat() const noexcept
		{
			return getRef().is_number_float();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isBinary() const noexcept
		{
			return getRef().is_binary();
		}

		template <class Derived>
		bool JSONMixin<Derived>::isInvalid() const noexcept
		{
			return getRef().is_discarded();
		}

		template <class Derived>
		JSONMixin<Derived>::operator bool() const noexcept
		{
			const detail::JSONBase& ref = getRef();
			return (!ref.is_discarded()) && (!ref.is_null());
		}

		template <class Derived>
		bool JSONMixin<Derived>::hasElement(const std::string_view key) const
		{
			return contains(key);
		}

		template <class Derived>
		bool JSONMixin<Derived>::hasElement(const StringView key) const
		{
			return contains(key);
		}

		template <class Derived>
		bool JSONMixin<Derived>::contains(std::string_view key) const
		{
			return getRef().contains(key);
		}

		template <class Derived>
		bool JSONMixin<Derived>::contains(StringView key) const
		{
			return getRef().contains(Unicode::ToUTF8(key));
		}

		template <class Derived>
		std::size_t JSONMixin<Derived>::size() const noexcept
		{
			return getRef().size();
		}

		template <class Derived>
		String JSONMixin<Derived>::getString() const
		{
			if (not isString())
			{
				throw Error{ "JSON::getString(): This JSON value is not a string" };
			}

			return Unicode::FromUTF8(getRef().template get<std::string_view>());
		}

		template <class Derived>
		std::string JSONMixin<Derived>::getUTF8() const
		{
			if (not isString())
			{
				throw Error{ "JSON::getUTF8(): This JSON value is not a string" };
			}

			return getRef().template get<std::string>();
		}

		template <class Derived>
		Blob JSONMixin<Derived>::getBinary() const
		{
			if (not isBinary())
			{
				throw Error{ "JSON::getBinary(): This JSON value is not a binary" };
			}

			const auto& binary = getRef().get_binary();
			return Blob{ binary.data(), binary.size() };
		}

		template <class Derived>
		JSONMixin<Derived>::const_iterator JSONMixin<Derived>::begin() const noexcept
		{
			return cbegin();
		}

		template <class Derived>
		JSONMixin<Derived>::const_iterator JSONMixin<Derived>::end() const noexcept
		{
			return cend();
		}

		template <class Derived>
		JSONMixin<Derived>::const_iterator JSONMixin<Derived>::cbegin() const noexcept
		{
			const detail::JSONBase& ref = getRef();
			return const_iterator{ ref.cbegin(), ref.is_array() ? 0 : detail::NotArray };
		}

		template <class Derived>
		JSONMixin<Derived>::const_iterator JSONMixin<Derived>::cend() const noexcept
		{
			const detail::JSONBase& ref = getRef();
			return const_iterator{ ref.cend(), ref.is_array() ? ref.size() : detail::NotArray };
		}

		template <class Derived>
		JSONConstRef JSONMixin<Derived>::operator [](std::string_view key) const
		{
			try
			{
				return JSONConstRef{ getRef().at(key) };
			}
			catch (const detail::JSONBase::exception& e)
			{
				throw Error{ fmt::format("JSON::operator []: {}", e.what()) };
			}
		}

		template <class Derived>
		JSONConstRef JSONMixin<Derived>::operator [](StringView key) const
		{
			try
			{
				return JSONConstRef{ getRef().at(Unicode::ToUTF8(key)) };
			}
			catch (const detail::JSONBase::exception& e)
			{
				throw Error{ fmt::format("JSON::operator []: {}", e.what()) };
			}
		}

		template <class Derived>
		JSONConstRef JSONMixin<Derived>::operator [](const std::size_t index) const
		{
			return JSONConstRef{ CheckedAt(getRef(), index) };
		}

		template <class Derived>
		JSON JSONMixin<Derived>::flatten() const
		{
			return JSON(getRef().flatten());
		}

		template <class Derived>
		JSON JSONMixin<Derived>::unflatten() const
		{
			return JSON(getRef().unflatten());
		}

		template <class Derived>
		String JSONMixin<Derived>::format(const char32 indent, const std::size_t spaceCount, const EnsureAscii ensureAscii) const
		{
			return Unicode::FromUTF8(formatUTF8(indent, spaceCount, ensureAscii));
		}

		template <class Derived>
		std::string JSONMixin<Derived>::formatUTF8(const char32 indent, const std::size_t spaceCount, const EnsureAscii ensureAscii) const
		{
			return getRef().dump(static_cast<int>(spaceCount), static_cast<char>(indent), ensureAscii.getBool());
		}

		template <class Derived>
		String JSONMixin<Derived>::formatMinified(const EnsureAscii ensureAscii) const
		{
			return Unicode::FromUTF8(formatUTF8Minified(ensureAscii));
		}

		template <class Derived>
		std::string JSONMixin<Derived>::formatUTF8Minified(const EnsureAscii ensureAscii) const
		{
			return getRef().dump(-1, ' ', ensureAscii.getBool());
		}

		template <class Derived>
		bool JSONMixin<Derived>::save(const FilePathView path, const char32 indent, const std::size_t spaceCount, const EnsureAscii ensureAscii) const
		{
			return SaveText(path, formatUTF8(indent, spaceCount, ensureAscii));
		}

		template <class Derived>
		bool JSONMixin<Derived>::saveMinified(const FilePathView path, const EnsureAscii ensureAscii) const
		{
			return SaveText(path, formatUTF8Minified(ensureAscii));
		}

		template <class Derived>
		Blob JSONMixin<Derived>::toBSON() const
		{
			return ToBlob(JSONBase::to_bson(getRef()));
		}

		template <class Derived>
		Blob JSONMixin<Derived>::toCBOR() const
		{
			return ToBlob(JSONBase::to_cbor(getRef()));
		}

		template <class Derived>
		Blob JSONMixin<Derived>::toMessagePack() const
		{
			return ToBlob(JSONBase::to_msgpack(getRef()));
		}

		template <class Derived>
		JSONArrayView<true> JSONMixin<Derived>::arrayView() const
		{
			if (getRef().is_array())
			{
				return JSONArrayView<true>{ getRef() };
			}
			else
			{
				throw Error{ "JSON::arrayView(): This JSON value is not an array" };
			}
		}

		template <class Derived>
		JSONMixin<Derived>::operator JSONConstRef() const noexcept
			requires std::same_as<Derived, JSON>
		{
			return JSONConstRef{ getRef() };
		}

		template <class Derived>
		JSONMixin<Derived>::operator JSONConstRef() const noexcept
			requires std::same_as<Derived, JSONRef>
		{
			return JSONConstRef{ getRef() };
		}

		template <class Derived>
		JSONMutableMixin<Derived>::iterator JSONMutableMixin<Derived>::begin() noexcept
		{
			JSONBase& json = getMutableRef();
			return iterator{ json.begin(), json.is_array() ? 0 : NotArray };
		}

		template <class Derived>
		JSONMutableMixin<Derived>::iterator JSONMutableMixin<Derived>::end() noexcept
		{
			JSONBase& json = getMutableRef();
			return iterator{ json.end(), json.is_array() ? json.size() : NotArray };
		}

		template <class Derived>
		JSONRef JSONMutableMixin<Derived>::operator [](std::string_view key)
		{
			try
			{
				return JSONRef{ getMutableRef()[key] };
			}
			catch (const detail::JSONBase::exception& e)
			{
				throw Error{ fmt::format("JSON::operator []: {}", e.what()) };
			}
		}

		template <class Derived>
		JSONRef JSONMutableMixin<Derived>::operator [](StringView key)
		{
			try
			{
				return JSONRef{ getMutableRef()[Unicode::ToUTF8(key)] };
			}
			catch (const detail::JSONBase::exception& e)
			{
				throw Error{ fmt::format("JSON::operator []: {}", e.what()) };
			}
		}

		template <class Derived>
		JSONRef JSONMutableMixin<Derived>::operator [](const std::size_t index)
		{
			return JSONRef{ CheckedAt(getMutableRef(), index) };
		}

		template <class Derived>
		void JSONMutableMixin<Derived>::push_back(JSON value)
		{
			PushBack(getMutableRef(), std::move(value.m_json));
		}

		template <class Derived>
		void JSONMutableMixin<Derived>::pop_back()
		{
			PopBack(getMutableRef());
		}

		template <class Derived>
		void JSONMutableMixin<Derived>::clear()
		{
			getMutableRef().clear();
		}

		template <class Derived>
		void JSONMutableMixin<Derived>::erase(const std::string_view key)
		{
			EraseKey(getMutableRef(), key);
		}

		template <class Derived>
		void JSONMutableMixin<Derived>::erase(const StringView key)
		{
			EraseKey(getMutableRef(), Unicode::ToUTF8(key));
		}

		template <class Derived>
		void JSONMutableMixin<Derived>::erase(const std::size_t index)
		{
			EraseIndex(getMutableRef(), index);
		}

		template <class Derived>
		JSONArrayView<false> JSONMutableMixin<Derived>::arrayView()
		{
			JSONBase& json = getMutableRef();

			if (json.is_array())
			{
				return JSONArrayView<false>{ json };
			}
			else
			{
				throw Error{ "JSON::arrayView(): This JSON value is not an array" };
			}
		}

		template class JSONMixin<JSON>;

		template class JSONMixin<JSONRef>;

		template class JSONMixin<JSONConstRef>;

		template class JSONMutableMixin<JSON>;

		template class JSONMutableMixin<JSONRef>;
	}

	JSON::JSON() : m_json() {}

	JSON::JSON(JSONValueType valueType) : m_json(static_cast<detail::JSONBase::value_t>(valueType)) {}

	JSON::JSON(json_base::initializer_list_t init, const bool typeDeduction, const JSONValueType manualType)
	try
		: m_json(init, typeDeduction, static_cast<detail::JSONBase::value_t>(manualType)) {}
	catch (const detail::JSONBase::exception& e)
	{
		throw Error{ fmt::format("JSON::JSON(): {}", e.what()) };
	}

	JSON::JSON(const JSON&) = default;

	JSON::JSON(JSON&&) noexcept = default;

	JSON::JSON(JSONRef ref) : m_json(ref.m_json_ref) {}

	JSON::JSON(JSONConstRef const_ref) : m_json(const_ref.m_json_ref) {}

	JSON& JSON::operator =(JSON other) noexcept
	{
		m_json = std::move(other).m_json;
		return *this;
	}

	void JSON::swap(JSON& other) noexcept
	{
		m_json.swap(other.m_json);
	}

	JSON JSON::Parse(std::string content, const FilePathView pathHint)
	{
		if (auto result = ParseResult(std::move(content), pathHint))
		{
			return std::move(*result);
		}

		return Invalid();
	}

	JSON JSON::Parse(const StringView content, const FilePathView pathHint)
	{
		return Parse(Unicode::ToUTF8(content), pathHint);
	}

	Result<JSON, Array<JSONParseErrorReason>> JSON::ParseResult(std::string content, const FilePathView pathHint)
	{
		auto result = ParseJSON(content, pathHint);

		if (not result)
		{
			return Err{ std::move(result).error() };
		}

		return JSON(std::move(*result));
	}

	Result<JSON, Array<JSONParseErrorReason>> JSON::ParseResult(const StringView content, const FilePathView pathHint)
	{
		return ParseResult(Unicode::ToUTF8(content), pathHint);
	}

	JSON JSON::Load(const FilePathView path)
	{
		if (auto result = LoadResult(path))
		{
			return std::move(*result);
		}

		return Invalid();
	}

	JSON JSON::Load(std::unique_ptr<IReader> reader)
	{
		if (auto result = LoadResult(std::move(reader)))
		{
			return std::move(*result);
		}

		return Invalid();
	}

	Result<JSON, Array<JSONParseErrorReason>> JSON::LoadResult(const FilePathView path)
	{
		BinaryFileReader reader{ path };
		return LoadJSON(reader, path);
	}

	Result<JSON, Array<JSONParseErrorReason>> JSON::LoadResult(std::unique_ptr<IReader> reader)
	{
		if (not reader)
		{
			return MakeJSONError(U"Reader is null.", {}, U"Provide a valid reader.");
		}

		return LoadJSON(*reader, U"");
	}

	JSON JSON::FromBSON(const Blob& bson, const AllowExceptions allowExceptions)
	{
		return FromBinary("FromBSON", allowExceptions, [&](const bool exceptions) { return json_base::from_bson(bson.begin(), bson.end(), true, exceptions); });
	}

	JSON JSON::FromCBOR(const Blob& cbor, const AllowExceptions allowExceptions)
	{
		return FromBinary("FromCBOR", allowExceptions, [&](const bool exceptions) { return json_base::from_cbor(cbor.begin(), cbor.end(), true, exceptions); });
	}

	JSON JSON::FromMessagePack(const Blob& msgpack, const AllowExceptions allowExceptions)
	{
		return FromBinary("FromMessagePack", allowExceptions, [&](const bool exceptions) { return json_base::from_msgpack(msgpack.begin(), msgpack.end(), true, exceptions); });
	}

	JSON JSON::EmptyArray()
	{
		return JSON(JSONValueType::Array);
	}

	JSON JSON::EmptyObject()
	{
		return JSON(JSONValueType::Object);
	}

	JSON JSON::EmptyBinary()
	{
		return JSON(JSONValueType::Binary);
	}

	JSON JSON::Invalid()
	{
		return JSON(JSONValueType::Invalid);
	}

	JSONRef& JSONRef::operator =(const JSONRef& other)
	{
		m_json_ref = other.m_json_ref;
		return *this;
	}

	const JSONRef& JSONRef::operator =(const JSONRef& other) const
	{
		m_json_ref = other.m_json_ref;
		return *this;
	}

	JSONRef& JSONRef::operator =(const JSON& other)
	{
		m_json_ref = other.m_json;
		return *this;
	}

	JSONRef& JSONRef::operator =(JSON&& other)
	{
		m_json_ref = std::move(other).m_json;
		return *this;
	}

	const JSONRef& JSONRef::operator =(const JSON& other) const
	{
		m_json_ref = other.m_json;
		return *this;
	}

	const JSONRef& JSONRef::operator =(JSON&& other) const
	{
		m_json_ref = std::move(other).m_json;
		return *this;
	}

	bool operator ==(JSONConstRef lhs, JSONConstRef rhs) noexcept
	{
		return lhs.m_json_ref == rhs.m_json_ref;
	}

	inline namespace Literals
	{
		inline namespace JSONLiterals
		{
			JSON operator ""_json(const char* str, const std::size_t length)
			{
				return JSON::Parse(std::string(str, length));
			}

			JSON operator ""_json(const char32_t* str, const std::size_t length)
			{
				return JSON::Parse(StringView{ str, length });
			}
		}
	}
}

void JSONSerializer<s3d::StringView>::to_json(s3d::JSON::json_base& j, const s3d::StringView& value)
{
	j = s3d::Unicode::ToUTF8(value);
}

void JSONSerializer<s3d::String>::to_json(s3d::JSON::json_base& j, const s3d::String& value)
{
	j = s3d::Unicode::ToUTF8(value);
}

void JSONSerializer<s3d::String>::from_json(const s3d::JSON::json_base& j, s3d::String& value)
{
	value = s3d::Unicode::FromUTF8(j.get<std::string>());
}

void JSONSerializer<std::u32string_view>::to_json(s3d::JSON::json_base& j, const std::u32string_view value)
{
	j = s3d::Unicode::ToUTF8(value);
}

void JSONSerializer<std::u32string>::to_json(s3d::JSON::json_base& j, const std::u32string& value)
{
	j = s3d::Unicode::ToUTF8(value);
}

void JSONSerializer<std::u32string>::from_json(const s3d::JSON::json_base& j, std::u32string& value)
{
	value = s3d::Unicode::UTF8ToUTF32(j.get<std::string_view>());
}

void JSONSerializer<const s3d::char32*>::to_json(s3d::JSON::json_base& j, const s3d::char32* value)
{
	j = s3d::Unicode::ToUTF8(value);
}
