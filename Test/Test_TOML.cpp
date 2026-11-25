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

# include "Siv3DTest.hpp"

namespace
{
	constexpr FilePathView ValidBasic = U"../../Test/data/toml/valid_basic.toml";
	constexpr FilePathView ValidAllTypes = U"../../Test/data/toml/valid_all_types.toml";
	constexpr FilePathView ValidNested = U"../../Test/data/toml/valid_nested.toml";
	constexpr FilePathView ValidUnicode = U"../../Test/data/toml/valid_unicode.toml";
	constexpr FilePathView ValidUTF8BOM = U"../../Test/data/toml/valid_utf8_bom.toml";
	constexpr FilePathView ValidEmpty = U"../../Test/data/toml/valid_empty.toml";
	constexpr FilePathView ValidCommentsWhitespace = U"../../Test/data/toml/valid_comments_whitespace.toml";
	constexpr FilePathView ValidTypeMismatchFixture = U"../../Test/data/toml/valid_type_mismatch_fixture.toml";

	constexpr FilePathView InvalidUnclosedArray = U"../../Test/data/toml/invalid_unclosed_array.toml";
	constexpr FilePathView InvalidDuplicateKey = U"../../Test/data/toml/invalid_duplicate_key.toml";
	constexpr FilePathView InvalidDuplicateTable = U"../../Test/data/toml/invalid_duplicate_table.toml";
	constexpr FilePathView InvalidInvalidDate = U"../../Test/data/toml/invalid_invalid_date.toml";
	constexpr FilePathView InvalidInvalidNumber = U"../../Test/data/toml/invalid_invalid_number.toml";
	constexpr FilePathView InvalidNonUTF8Latin1 = U"../../Test/data/toml/invalid_non_utf8_latin1.toml";
	constexpr FilePathView Nonexistent = U"../../Test/data/toml/nonexistent.toml";
}

TEST_CASE("TOML.default")
{
	const TOML toml;

	CHECK(not toml);
	CHECK(toml.isInvalid());
	CHECK(not toml.isValid());
	CHECK((toml.getType()) == (TOMLValueType::Invalid));
	CHECK((toml.type()) == (TOMLValueType::Invalid));

	CHECK(not toml.isBool());
	CHECK(not toml.isInt());
	CHECK(not toml.isFloat());
	CHECK(not toml.isString());
	CHECK(not toml.isArray());
	CHECK(not toml.isTable());

	CHECK(toml.format().isEmpty());
	CHECK(toml.formatUTF8().empty());
}

TEST_CASE("TOML.Load.basic")
{
	const TOML toml = TOML::Load(ValidBasic);

	REQUIRE(toml);
	CHECK(toml.isTable());
	CHECK((toml.getType()) == (TOMLValueType::Table));
	CHECK((toml.size()) == (5u));

	CHECK(toml.contains(U"title"));
	CHECK(toml.contains(U"server"));
	CHECK(toml.contains(U"database"));
	CHECK(toml.contains(U"features"));
	CHECK(toml.contains(U"users"));
	CHECK(not toml.contains(U"missing"));

	CHECK((toml[U"title"].get<String>()) == (U"TOML test"));

	const TOML server = toml[U"server"];
	REQUIRE(server);
	CHECK(server.isTable());
	CHECK((server[U"host"].get<String>()) == (U"localhost"));
	CHECK((server[U"host"].getUTF8()) == (std::string{ "localhost" }));
	CHECK((server[U"port"].get<int32>()) == (8080));
	CHECK((server[U"enabled"].get<bool>()) == (true));

	const TOML database = toml[U"database"];
	REQUIRE(database);
	CHECK((database[U"user"].get<String>()) == (U"test_user"));
	CHECK((database[U"password"].get<String>()) == (U"secret"));
	CHECK((database[U"max_connections"].get<int32>()) == (10));
	CHECK((database[U"timeout_seconds"].get<double>()) == (30.5));

	const TOML features = toml[U"features"];
	REQUIRE(features);
	CHECK(features[U"items"].getArray<String>() == Array<String>{ U"login", U"search", U"export" });
	CHECK(features[U"retry_counts"].getArray<int32>() == Array<int32>{ 1, 2, 3 });

	const TOML users = toml[U"users"];
	REQUIRE(users);
	CHECK(users.isArray());
	CHECK((users.size()) == (2u));
	CHECK((users[0][U"name"].get<String>()) == (U"Alice"));
	CHECK((users[0][U"admin"].get<bool>()) == (true));
	CHECK((users[1][U"name"].get<String>()) == (U"Bob"));
	CHECK((users[1][U"admin"].get<bool>()) == (false));
}

TEST_CASE("TOML.Parse")
{
	{
		const TOML toml = TOML::Parse(U"title = \"Siv3D\"\nvalue = 42\n");

		REQUIRE(toml);
		CHECK(toml.isTable());
		CHECK((toml[U"title"].get<String>()) == (U"Siv3D"));
		CHECK((toml[U"value"].get<int32>()) == (42));
	}

	{
		const TOML toml = TOML::Parse(std::string{ "title = \"Siv3D\"\nvalue = 42\n" });

		REQUIRE(toml);
		CHECK(toml.isTable());
		CHECK((toml[U"title"].get<String>()) == (U"Siv3D"));
		CHECK((toml[U"value"].get<int32>()) == (42));
	}

	{
		const TOML toml = TOML::Parse(U"");

		REQUIRE(toml);
		CHECK(toml.isTable());
		CHECK(toml.isEmptyTable());
		CHECK((toml.size()) == (0u));
	}

	{
		const TOML toml = TOML::Parse(U"title = \"broken\"\narray = [1, 2, 3\n");

		CHECK(not toml);
		CHECK(toml.isInvalid());
	}
}

TEST_CASE("TOML.LoadResult.ParseResult")
{
	{
		const auto result = TOML::LoadResult(ValidBasic);
		REQUIRE(result);
	}

	{
		const auto result = TOML::ParseResult(U"title = \"Siv3D\"\nvalue = 42\n");
		REQUIRE(result);
	}

	{
		const auto result = TOML::LoadResult(Nonexistent);
		REQUIRE(not result);
		REQUIRE(not result.error().isEmpty());
		CHECK(not result.error().front().title.isEmpty());
	}

	{
		const auto result = TOML::ParseResult(U"title = \"broken\"\narray = [1, 2, 3\n");
		REQUIRE(not result);
		REQUIRE(not result.error().isEmpty());
		CHECK(not result.error().front().title.isEmpty());
	}
}

TEST_CASE("TOML.Load.invalid")
{
	constexpr std::array InvalidFiles
	{
		InvalidUnclosedArray,
		InvalidDuplicateKey,
		InvalidDuplicateTable,
		InvalidInvalidDate,
		InvalidInvalidNumber,
	};

	for (const auto& path : InvalidFiles)
	{
		{
			const TOML toml = TOML::Load(path);
			CHECK(not toml);
			CHECK(toml.isInvalid());
		}

		{
			const auto result = TOML::LoadResult(path);
			CHECK(not result);
			if (not result)
			{
				CHECK(not result.error().isEmpty());
			}
		}
	}

	{
		const TOML toml = TOML::Load(InvalidNonUTF8Latin1);
		CHECK(not toml);
		CHECK(toml.isInvalid());
	}

	{
		const auto result = TOML::LoadResult(InvalidNonUTF8Latin1);
		CHECK(not result);
	}
}

TEST_CASE("TOML.types")
{
	const TOML toml = TOML::Load(ValidAllTypes);
	REQUIRE(toml);

	CHECK(toml[U"bool_true"].isBool());
	CHECK(toml[U"bool_false"].isBool());
	CHECK((toml[U"bool_true"].get<bool>()) == (true));
	CHECK((toml[U"bool_false"].get<bool>()) == (false));

	CHECK(toml[U"int_zero"].isInt());
	CHECK(toml[U"int_positive"].isInt());
	CHECK(toml[U"int_negative"].isInt());
	CHECK(toml[U"int_with_underscores"].isInt());
	CHECK((toml[U"int_zero"].get<int32>()) == (0));
	CHECK((toml[U"int_positive"].get<int32>()) == (123456));
	CHECK((toml[U"int_negative"].get<int32>()) == (-98765));
	CHECK((toml[U"int_with_underscores"].get<int32>()) == (1'000'000));

	CHECK(toml[U"float_plain"].isFloat());
	CHECK(toml[U"float_negative"].isFloat());
	CHECK(toml[U"float_exponent"].isFloat());
	CHECK(toml[U"float_inf"].isFloat());
	CHECK(toml[U"float_nan"].isFloat());
	CHECK(toml[U"float_plain"].get<double>() == Test::Approx(3.14159));
	CHECK(toml[U"float_negative"].get<double>() == Test::Approx(-0.25));
	CHECK(std::isinf(toml[U"float_inf"].get<double>()));
	CHECK(std::isnan(toml[U"float_nan"].get<double>()));

	CHECK(toml[U"string_basic"].isString());
	CHECK(toml[U"string_unicode"].isString());
	CHECK((toml[U"string_basic"].get<String>()) == (U"hello"));
	CHECK((toml[U"string_unicode"].get<String>()) == (U"こんにちは Siv3D 🌏"));

	CHECK(toml[U"local_date"].isLocalDate());
	CHECK((toml[U"local_date"].get<Date>()) == (Date{ 2026, 5, 4 }));

	CHECK(toml[U"local_time"].isLocalTime());
	{
		const TOML::LocalTime time = toml[U"local_time"].getLocalTime();
		CHECK((time.hour) == (12));
		CHECK((time.minute) == (34));
		CHECK((time.second) == (56));
		CHECK((time.millisecond) == (789));
		CHECK((time.microsecond) == (123));
		CHECK((time.nanosecond) == (456));
	}

	CHECK(toml[U"local_datetime"].isLocalDateTime());
	{
		const TOML::LocalDateTime dateTime = toml[U"local_datetime"].getLocalDateTime();
		CHECK((dateTime.date) == (Date{ 2026, 5, 4 }));
		CHECK((dateTime.time.hour) == (12));
		CHECK((dateTime.time.minute) == (34));
		CHECK((dateTime.time.second) == (56));
		CHECK((dateTime.time.millisecond) == (789));
	}

	CHECK(toml[U"offset_datetime_utc"].isOffsetDateTime());
	{
		const TOML::OffsetDateTime dateTime = toml[U"offset_datetime_utc"].getOffsetDateTime();
		CHECK((dateTime.localDateTime.date) == (Date{ 2026, 5, 4 }));
		CHECK((dateTime.localDateTime.time.hour) == (12));
		CHECK((dateTime.localDateTime.time.minute) == (34));
		CHECK((dateTime.localDateTime.time.second) == (56));
		CHECK((dateTime.offsetMinutes) == (0));
	}

	CHECK(toml[U"offset_datetime_jst"].isOffsetDateTime());
	{
		const TOML::OffsetDateTime dateTime = toml[U"offset_datetime_jst"].getOffsetDateTime();
		CHECK((dateTime.localDateTime.date) == (Date{ 2026, 5, 4 }));
		CHECK((dateTime.localDateTime.time.hour) == (21));
		CHECK((dateTime.localDateTime.time.minute) == (34));
		CHECK((dateTime.localDateTime.time.second) == (56));
		CHECK((dateTime.offsetMinutes) == (9 * 60));
	}
}

TEST_CASE("TOML.operator_at")
{
	const TOML toml = TOML::Load(ValidBasic);
	REQUIRE(toml);

	CHECK(toml[U"server"]);
	CHECK(toml[U"server"][U"host"]);
	CHECK((toml[U"server"][U"host"].get<String>()) == (U"localhost"));

	CHECK_THROWS_AS((void)toml[U"missing"], Error);
	CHECK_THROWS_AS((void)toml[U"server"][U"missing"], Error);
	CHECK_THROWS_AS((void)toml[U"server"][0], Error);
	CHECK_THROWS_AS((void)toml[U"users"][999], Error);
	CHECK_THROWS_AS((void)toml[U"title"][U"missing"], Error);

	CHECK((toml.at(U"server").at(U"host").get<String>()) == (U"localhost"));
	CHECK((toml.at(U"users").at(0).at(U"name").get<String>()) == (U"Alice"));

	CHECK_THROWS_AS((void)toml.at(U"missing"), Error);
	CHECK_THROWS_AS((void)toml[U"server"].at(0), Error);
	CHECK_THROWS_AS((void)toml[U"users"].at(999), Error);
	CHECK_THROWS_AS((void)toml[U"title"].at(U"missing"), Error);
}

TEST_CASE("TOML.getOpt_getOr_get")
{
	const TOML toml = TOML::Load(ValidTypeMismatchFixture);
	REQUIRE(toml);

	{
		const Optional<int32> opt = toml[U"as_int"].getOpt<int32>();
		REQUIRE(opt);
		CHECK((*opt) == (123));
	}

	{
		const Optional<String> opt = toml[U"as_string_number"].getOpt<String>();
		REQUIRE(opt);
		CHECK((*opt) == (U"123"));
	}

	CHECK(not toml[U"as_string_number"].getOpt<int32>());
	CHECK(not toml[U"as_float"].getOpt<int32>());
	CHECK(not toml[U"as_bool"].getOpt<int32>());

	CHECK((toml[U"as_string_number"].getOr<int32>(456)) == (456));
	CHECK((toml[U"as_int"].getOr<String>(U"default")) == (U"default"));

	CHECK_THROWS_AS((void)toml[U"as_string_number"].get<int32>(), Error);
	CHECK_THROWS_AS((void)toml[U"as_float"].get<int32>(), Error);
}

TEST_CASE("TOML.getParsed")
{
	const TOML toml = TOML::Load(ValidTypeMismatchFixture);
	REQUIRE(toml);

	{
		const Optional<int32> opt = toml[U"as_string_number"].getParsedOpt<int32>();
		REQUIRE(opt);
		CHECK((*opt) == (123));
	}

	CHECK(not toml[U"as_int"].getParsedOpt<int32>());

	CHECK((toml[U"as_string_number"].getParsed<int32>()) == (123));
	CHECK((toml[U"as_int"].getParsedOr<int32>(456)) == (456));
	CHECK_THROWS_AS((void)toml[U"as_int"].getParsed<int32>(), Error);
}

TEST_CASE("TOML.getArray")
{
	const TOML toml = TOML::Load(ValidTypeMismatchFixture);
	REQUIRE(toml);

	CHECK(toml[U"as_array_ints"].getArray<int32>() == Array<int32>{ 1, 2, 3 });
	CHECK(not toml[U"as_array_mixed"].getArrayOpt<int32>());
	CHECK(not toml[U"as_int"].getArrayOpt<int32>());

	CHECK_THROWS_AS((void)toml[U"as_array_mixed"].getArray<int32>(), Error);
	CHECK_THROWS_AS((void)toml[U"as_int"].getArray<int32>(), Error);

	const TOML basic = TOML::Load(ValidBasic);
	REQUIRE(basic);

	const Array<TOML> users = basic[U"users"].getArray<TOML>();
	REQUIRE((users.size()) == (2u));
	CHECK((users[0][U"name"].get<String>()) == (U"Alice"));
	CHECK((users[1][U"name"].get<String>()) == (U"Bob"));
}

TEST_CASE("TOML.arrayView")
{
	const TOML toml = TOML::Load(ValidBasic);
	REQUIRE(toml);

	Array<String> items;
	for (const auto item : toml[U"features"][U"items"].arrayView())
	{
		items.push_back(item.get<String>());
	}
	CHECK(items == Array<String>{ U"login", U"search", U"export" });

	Array<String> userNames;
	for (const auto user : toml[U"users"].arrayView())
	{
		userNames.push_back(user[U"name"].get<String>());
	}
	CHECK(userNames == Array<String>{ U"Alice", U"Bob" });

	CHECK_THROWS_AS((void)toml[U"server"].arrayView(), Error);
	CHECK_THROWS_AS((void)toml[U"missing"].arrayView(), Error);
}

TEST_CASE("TOML.tableView")
{
	const TOML toml = TOML::Load(ValidBasic);
	REQUIRE(toml);

	Array<String> serverKeys;
	for (const auto member : toml[U"server"].tableView())
	{
		serverKeys.push_back(member.name);
	}
	CHECK(serverKeys == Array<String>{ U"host", U"port", U"enabled" });

	Array<String> rootKeys;
	for (const auto member : toml.tableView())
	{
		rootKeys.push_back(member.name);
	}
	CHECK(rootKeys == Array<String>{ U"title", U"server", U"database", U"features", U"users" });

	CHECK_THROWS_AS((void)toml[U"users"].tableView(), Error);
	CHECK_THROWS_AS((void)toml[U"missing"].tableView(), Error);
}

TEST_CASE("TOML.lifetime")
{
	const TOML server = TOML::Load(ValidBasic)[U"server"];

	REQUIRE(server);
	CHECK(server.isTable());
	CHECK((server[U"host"].get<String>()) == (U"localhost"));
	CHECK((server[U"port"].get<int32>()) == (8080));

	const TOML firstUser = TOML::Load(ValidBasic)[U"users"][0];

	REQUIRE(firstUser);
	CHECK(firstUser.isTable());
	CHECK((firstUser[U"name"].get<String>()) == (U"Alice"));
	CHECK((firstUser[U"admin"].get<bool>()) == (true));
}

TEST_CASE("TOML.nested")
{
	const TOML toml = TOML::Load(ValidNested);
	REQUIRE(toml);

	CHECK((toml[U"window"][U"title"].get<String>()) == (U"Game"));
	CHECK(toml[U"window"][U"size"].getArray<int32>() == Array<int32>{ 1280, 720 });
	CHECK((toml[U"window"][U"fullscreen"].get<bool>()) == (false));

	CHECK((toml[U"graphics"][U"vsync"].get<bool>()) == (true));
	CHECK(toml[U"graphics"][U"clear_color"].isTable());
	CHECK(toml[U"graphics"][U"clear_color"][U"r"].get<double>() == Test::Approx(0.1));
	CHECK(toml[U"graphics"][U"clear_color"][U"g"].get<double>() == Test::Approx(0.2));
	CHECK(toml[U"graphics"][U"clear_color"][U"b"].get<double>() == Test::Approx(0.3));
	CHECK(toml[U"graphics"][U"clear_color"][U"a"].get<double>() == Test::Approx(1.0));

	CHECK((toml[U"stages"].size()) == (2u));
	CHECK((toml[U"stages"][0][U"name"].get<String>()) == (U"forest"));
	CHECK((toml[U"stages"][0][U"difficulty"].get<int32>()) == (1));
	CHECK((toml[U"stages"][0][U"spawn_points"].size()) == (3u));
	CHECK(toml[U"stages"][0][U"spawn_points"][0].getArray<int32>() == Array<int32>{ 0, 0 });

	CHECK((toml[U"stages"][1][U"name"].get<String>()) == (U"desert"));
	CHECK((toml[U"stages"][1][U"difficulty"].get<int32>()) == (3));
	CHECK((toml[U"stages"][1][U"spawn_points"].size()) == (2u));
	CHECK(toml[U"stages"][1][U"spawn_points"][1].getArray<int32>() == Array<int32>{ 200, 160 });
}

TEST_CASE("TOML.unicode_bom_empty")
{
	{
		const TOML toml = TOML::Load(ValidUnicode);
		REQUIRE(toml);
		CHECK((toml[U"title"].get<String>()) == (U"日本語タイトル"));
		CHECK((toml[U"description"].get<String>()) == (U"Unicode / 絵文字 / 記号のテスト: あいうえお、αβγ、🎮"));
		CHECK((toml[U"names"][U"player"].get<String>()) == (U"太郎"));
		CHECK((toml[U"messages"][U"game_over"].get<String>()) == (U"ゲームオーバー"));
	}

	{
		const TOML toml = TOML::Load(ValidUTF8BOM);
		REQUIRE(toml);
		CHECK((toml[U"title"].get<String>()) == (U"UTF-8 BOM test"));
		CHECK((toml[U"value"].get<int32>()) == (42));
		CHECK((toml[U"message"].get<String>()) == (U"BOM 付き UTF-8"));
	}

	{
		const TOML toml = TOML::Load(ValidEmpty);
		REQUIRE(toml);
		CHECK(toml.isTable());
		CHECK(toml.isEmptyTable());
		CHECK((toml.size()) == (0u));
	}

	{
		const TOML toml = TOML::Load(ValidCommentsWhitespace);
		REQUIRE(toml);
		CHECK(toml[U"empty_table"].isTable());
		CHECK(toml[U"empty_table"].isEmptyTable());
		CHECK((toml[U"config"][U"value"].get<int32>()) == (42));
		CHECK(toml[U"config"][U"array"].getArray<int32>() == Array<int32>{ 1, 2, 3 });
	}
}

TEST_CASE("TOML.format")
{
	const TOML toml = TOML::Load(ValidBasic);
	REQUIRE(toml);

	const String formatted = toml.format();
	const std::string formattedUTF8 = toml.formatUTF8();

	CHECK(not formatted.isEmpty());
	CHECK(not formattedUTF8.empty());

	CHECK(formatted.contains(U"title"));
	CHECK(formatted.contains(U"server"));
	CHECK(formatted.contains(U"users"));
}

TEST_CASE("TOML.path")
{
	const TOML toml = TOML::Load(ValidBasic);
	REQUIRE(toml);

	CHECK((toml.path()) == (U"$"));
	CHECK((toml[U"server"].path()) == (U"$[\"server\"]"));
	CHECK((toml[U"server"][U"host"].path()) == (U"$[\"server\"][\"host\"]"));
	CHECK((toml.at(U"users").at(1).at(U"name").path()) == (U"$[\"users\"][1][\"name\"]"));

	Array<String> userPaths;
	for (const auto user : toml[U"users"].arrayView())
	{
		userPaths.push_back(user.path());
	}
	CHECK(userPaths == Array<String>{ U"$[\"users\"][0]", U"$[\"users\"][1]" });

	Array<String> memberPaths;
	for (const auto member : toml[U"server"].tableView())
	{
		memberPaths.push_back(member.value.path());
	}
	CHECK(memberPaths == Array<String>{ U"$[\"server\"][\"host\"]", U"$[\"server\"][\"port\"]", U"$[\"server\"][\"enabled\"]" });

	const TOML server = toml[U"server"];
	CHECK((server.path()) == (U"$"));
	CHECK((server[U"port"].path()) == (U"$[\"port\"]"));

	TOML mutableToml = toml;
	CHECK((mutableToml[U"users"][0][U"admin"].path()) == (U"$[\"users\"][0][\"admin\"]"));
}

TEST_CASE("TOML.errorMessages")
{
	using Catch::Matchers::ContainsSubstring;

	const TOML toml = TOML::Load(ValidBasic);
	REQUIRE(toml);

	CHECK_THROWS_WITH(toml[U"server"][U"missing"], ContainsSubstring("key was not found at $[\"server\"]"));
	CHECK_THROWS_WITH(toml[U"server"][U"host"][U"x"], ContainsSubstring("value at $[\"server\"][\"host\"] is not a table"));
	CHECK_THROWS_WITH(toml.at(U"users").at(999), ContainsSubstring("index out of range at $[\"users\"]. Size: 2"));
	CHECK_THROWS_WITH(toml[U"server"][0], ContainsSubstring("value at $[\"server\"] is not an array"));
	CHECK_THROWS_WITH(toml[U"server"][U"port"].get<String>(), ContainsSubstring("failed at $[\"server\"][\"port\"]. Actual type: Int, value: \"8080\""));
	CHECK_THROWS_WITH(toml[U"users"].getArray<int32>(), ContainsSubstring("element conversion failed at $[\"users\"][0]"));
	CHECK_THROWS_WITH(toml[U"server"].arrayView(), ContainsSubstring("value at $[\"server\"] is not an array"));
}

TEST_CASE("TOML.ownership")
{
	const TOML original = TOML::Parse(U"a = 1\n[t]\nb = 2\n");
	REQUIRE(original);

	{
		TOML copy = original;
		copy[U"a"] = 10;
		copy[U"t"][U"b"] = 20;
		CHECK((original[U"a"].get<int32>()) == (1));
		CHECK((original[U"t"][U"b"].get<int32>()) == (2));
		CHECK((copy[U"a"].get<int32>()) == (10));
		CHECK((copy[U"t"][U"b"].get<int32>()) == (20));
	}

	{
		const TOML brace{ original };
		CHECK(brace.isTable());
		CHECK(brace == original);
	}

	{
		TOML document = original;
		TOMLRef alias = document[U"t"];
		alias[U"b"] = 3;
		CHECK((document[U"t"][U"b"].get<int32>()) == (3));

		TOML sub = document[U"t"];
		sub[U"b"] = 4;
		CHECK((document[U"t"][U"b"].get<int32>()) == (3));
		CHECK((sub[U"b"].get<int32>()) == (4));

		const TOMLConstRef constAlias = document[U"t"];
		CHECK((constAlias[U"b"].get<int32>()) == (3));
	}

	{
		const Optional<TOML> copy = original[U"t"].getOpt<TOML>();
		REQUIRE(copy);
		CHECK((copy->path()) == (U"$"));
		CHECK((*copy) == (original[U"t"]));
	}

	{
		TOML document = original;
		CHECK(not document.contains(U"missing"));
		CHECK(document[U"missing"].isInvalid());
		CHECK(document.contains(U"missing"));
		CHECK_THROWS_AS((void)document.format(), Error);

		document[U"missing"] = U"filled";
		CHECK((document[U"missing"].get<String>()) == (U"filled"));
		CHECK(not document.format().isEmpty());
	}

	static_assert(not std::is_copy_assignable_v<TOMLConstRef>);
	static_assert(std::is_convertible_v<TOMLRef, TOMLConstRef>);
	static_assert(std::is_convertible_v<const TOML&, TOMLConstRef>);
	static_assert(std::is_same_v<decltype(std::declval<const TOML&>()[U"key"]), TOMLConstRef>);
	static_assert(std::is_same_v<decltype(std::declval<TOML&>()[U"key"]), TOMLRef>);
	static_assert(std::is_same_v<decltype(std::declval<const TOMLRef&>()[U"key"]), TOMLConstRef>);
	static_assert(std::is_same_v<decltype(std::declval<TOMLRef&>()[0]), TOMLRef>);
}

TEST_CASE("TOML.mutation")
{
	{
		TOML toml;
		CHECK(toml.isInvalid());

		toml[U"title"] = U"Siv3D";
		CHECK(toml.isTable());
		toml[U"window"][U"width"] = 1280;
		toml[U"window"][U"height"] = 720;
		toml[U"values"].push_back(1);
		toml[U"values"].push_back(2.5);
		toml[U"values"].push_back(U"three");

		CHECK((toml[U"title"].get<String>()) == (U"Siv3D"));
		CHECK((toml[U"window"][U"width"].get<int32>()) == (1280));
		CHECK((toml[U"values"].size()) == (3u));
		CHECK((toml[U"values"][1].get<double>()) == (2.5));
		CHECK((toml[U"values"][2].get<String>()) == (U"three"));

		Array<String> keys;
		for (const auto member : toml.tableView())
		{
			keys.push_back(member.name);
		}
		CHECK(keys == Array<String>{ U"title", U"window", U"values" });

		const TOML reparsed = TOML::Parse(toml.format());
		REQUIRE(reparsed);
		CHECK((reparsed[U"window"][U"height"].get<int32>()) == (720));
		CHECK((reparsed[U"values"][2].get<String>()) == (U"three"));
	}

	{
		TOML toml = TOML::Parse(U"a = [1, 2, 3]\nb = 1\n[t]\nx = 1\ny = 2\n");
		REQUIRE(toml);

		toml[U"a"].pop_back();
		CHECK(toml[U"a"].getArray<int32>() == Array<int32>{ 1, 2 });
		toml[U"a"].erase(0);
		CHECK(toml[U"a"].getArray<int32>() == Array<int32>{ 2 });
		toml[U"a"][0] = 5;
		CHECK(toml[U"a"].getArray<int32>() == Array<int32>{ 5 });
		toml.at(U"a").at(0) = 6;
		CHECK(toml[U"a"].getArray<int32>() == Array<int32>{ 6 });

		toml[U"t"].erase(U"x");
		toml[U"t"].erase(U"missing");
		CHECK((toml[U"t"].size()) == (1u));
		CHECK(not toml[U"t"].contains(U"x"));

		toml[U"t"].clear();
		CHECK(toml[U"t"].isEmptyTable());
		toml[U"a"].clear();
		CHECK(toml[U"a"].isEmptyArray());

		toml[U"t"] = toml[U"b"];
		CHECK((toml[U"t"].get<int32>()) == (1));

		for (int32 i = 0; i < 64; ++i)
		{
			toml[U"copy{}"_fmt(i)] = TOML{ toml[U"b"] };
		}
		CHECK((toml[U"copy63"].get<int32>()) == (1));
		for (int32 i = 0; i < 64; ++i)
		{
			toml.erase(U"copy{}"_fmt(i));
		}

		toml[U"n"] = TOML::EmptyArray();
		toml[U"n"].push_back(TOML::EmptyTable());
		toml[U"n"][0][U"k"] = true;
		CHECK((toml[U"n"][0][U"k"].get<bool>()) == (true));

		toml[U"c"] = toml;
		CHECK(toml[U"c"].isTable());
		CHECK((toml[U"c"][U"b"].get<int32>()) == (1));

		toml[U"s1"] = 1;
		toml[U"s2"] = 2;
		swap(toml[U"s1"], toml[U"s2"]);
		CHECK((toml[U"s1"].get<int32>()) == (2));
		CHECK((toml[U"s2"].get<int32>()) == (1));
	}

	{
		TOML toml = TOML::Parse(U"a = [1, 2]\nb = 1\n");
		REQUIRE(toml);

		for (auto element : toml[U"a"].arrayView())
		{
			element = (element.get<int32>() * 10);
		}
		CHECK(toml[U"a"].getArray<int32>() == Array<int32>{ 10, 20 });

		for (auto member : toml.tableView())
		{
			if (member.name == U"b")
			{
				member.value = U"replaced";
			}
		}
		CHECK((toml[U"b"].get<String>()) == (U"replaced"));
	}

	{
		TOML toml = TOML::Parse(U"a = [1]\nb = 1\nt = {}\n");
		REQUIRE(toml);

		CHECK_THROWS_AS((void)toml[U"b"][U"x"], Error);
		CHECK_THROWS_AS((void)toml[U"a"][1], Error);
		CHECK_THROWS_AS((void)toml.at(U"missing"), Error);
		CHECK(not toml.contains(U"missing"));
		CHECK_THROWS_AS(toml[U"b"].push_back(1), Error);
		CHECK_THROWS_AS(toml[U"t"].pop_back(), Error);
		CHECK_THROWS_AS(toml[U"b"].clear(), Error);
		CHECK_THROWS_AS(toml[U"a"].erase(U"x"), Error);
		CHECK_THROWS_AS(toml[U"a"].erase(1), Error);
		CHECK_THROWS_AS(toml[U"t"].erase(0), Error);
		CHECK_THROWS_AS((void)toml[U"b"].arrayView(), Error);
		CHECK_THROWS_AS((void)toml[U"a"].tableView(), Error);

		toml[U"a"].pop_back();
		CHECK_THROWS_AS(toml[U"a"].pop_back(), Error);
	}
}

TEST_CASE("TOML.construction")
{
	CHECK(TOML{ true }.isBool());
	CHECK((TOML{ 42 }.get<int32>()) == (42));
	CHECK((TOML{ int64{ -1 } }.get<int64>()) == (-1));
	CHECK((TOML{ uint64{ 9223372036854775807ull } }.get<int64>()) == (9223372036854775807ll));
	CHECK_THROWS_AS(TOML{ uint64{ 9223372036854775808ull } }, Error);
	CHECK((TOML{ 0.5 }.get<double>()) == (0.5));
	CHECK((TOML{ 0.5f }.get<double>()) == (0.5));
	CHECK((TOML{ "utf8" }.get<String>()) == (U"utf8"));
	CHECK((TOML{ std::string{ "utf8" } }.get<String>()) == (U"utf8"));
	CHECK((TOML{ std::string_view{ "utf8" } }.get<String>()) == (U"utf8"));
	CHECK((TOML{ U"こんにちは" }.get<String>()) == (U"こんにちは"));
	CHECK((TOML{ StringView{ U"view" } }.get<String>()) == (U"view"));
	CHECK((TOML{ String{ U"string" } }.get<String>()) == (U"string"));
	CHECK((TOML{ Date{ 2026, 5, 4 } }.get<Date>()) == (Date{ 2026, 5, 4 }));

	{
		const TOML::LocalTime time{ 12, 34, 56, 789, 123, 456 };
		const TOML::LocalTime result = TOML{ time }.getLocalTime();
		CHECK((result.hour) == (12));
		CHECK((result.minute) == (34));
		CHECK((result.second) == (56));
		CHECK((result.millisecond) == (789));
		CHECK((result.microsecond) == (123));
		CHECK((result.nanosecond) == (456));
	}

	{
		const TOML::LocalDateTime dateTime{ Date{ 2026, 5, 4 }, TOML::LocalTime{ 1, 2, 3 } };
		const TOML::LocalDateTime result = TOML{ dateTime }.getLocalDateTime();
		CHECK((result.date) == (Date{ 2026, 5, 4 }));
		CHECK((result.time.hour) == (1));
	}

	{
		const TOML::OffsetDateTime dateTime{ TOML::LocalDateTime{ Date{ 2026, 5, 4 }, TOML::LocalTime{ 21, 34, 56 } }, (9 * 60) };
		const TOML::OffsetDateTime result = TOML{ dateTime }.getOffsetDateTime();
		CHECK((result.localDateTime.time.hour) == (21));
		CHECK((result.offsetMinutes) == (9 * 60));

		const TOML::OffsetDateTime negative{ TOML::LocalDateTime{ Date{ 2026, 5, 4 }, TOML::LocalTime{ 1, 0, 0 } }, -(3 * 60 + 30) };
		CHECK((TOML{ negative }.getOffsetDateTime().offsetMinutes) == (-(3 * 60 + 30)));
	}

	{
		const TOML array{ Array<int32>{ 1, 2, 3 } };
		CHECK(array.getArray<int32>() == Array<int32>{ 1, 2, 3 });

		const TOML mixed{ Array<TOML>{ 1, U"x", TOML::EmptyTable() } };
		CHECK((mixed.size()) == (3u));
		CHECK(mixed[1].isString());
		CHECK(mixed[2].isEmptyTable());

		const TOML nested{ Array<Array<int32>>{ { 1 }, { 2, 3 } } };
		CHECK(nested[1].getArray<int32>() == Array<int32>{ 2, 3 });
	}

	{
		const TOML table{ HashTable<String, int32>{ { U"a", 1 }, { U"b", 2 } } };
		CHECK(table.isTable());
		CHECK((table.size()) == (2u));
		CHECK((table[U"a"].get<int32>()) == (1));
		CHECK((table[U"b"].get<int32>()) == (2));
	}

	{
		const TOML table{ { U"name", U"Siv3D" }, { U"size", 2 }, { U"nested", { { U"flag", true } } } };
		Array<String> keys;
		for (const auto member : table.tableView())
		{
			keys.push_back(member.name);
		}
		CHECK(keys == Array<String>{ U"name", U"size", U"nested" });
		CHECK((table[U"nested"][U"flag"].get<bool>()) == (true));

		CHECK_THROWS_AS((TOML{ { U"a", 1 }, { U"a", 2 } }), Error);
	}

	CHECK(TOML::EmptyArray().isEmptyArray());
	CHECK(TOML::EmptyTable().isEmptyTable());
	CHECK(TOML::Invalid().isInvalid());

	static_assert(not std::is_constructible_v<TOML, char>);
	static_assert(not std::is_constructible_v<TOML, char32>);
	static_assert(not std::is_constructible_v<TOML, int*>);
	static_assert(std::is_convertible_v<int32, TOML>);
	static_assert(std::is_convertible_v<const char32*, TOML>);
}
