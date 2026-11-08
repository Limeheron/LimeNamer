#include "utf_test.h"

TEST(UTF, decode_utf8_accepts_valid_text)
{
    EXPECT_EQ(UTF::decode_utf8(""), std::u32string());
    EXPECT_EQ(UTF::decode_utf8("A"), U"A");
    EXPECT_EQ(UTF::decode_utf8("\xC3\xA9"), U"é");
    EXPECT_EQ(UTF::decode_utf8("\xED\x95\x9C"), U"한");
    EXPECT_EQ(UTF::decode_utf8("\xF0\x9F\x98\x80"), U"\U0001F600");
    EXPECT_EQ(UTF::decode_utf8("a\xC3\xA9\xED\x95\x9C\xF0\x9F\x98\x80z"),
        U"aé한\U0001F600z");
    EXPECT_EQ(UTF::decode_utf8(std::string_view("a\0b", 3)), std::u32string(U"a\0b", 3));
}

TEST(UTF, decode_utf8_rejects_invalid_text)
{
    const std::string_view samples[] = {
        "\x80",
        "\xBF",
        "\xF8\x88\x80\x80\x80",
        "\xFF",
        "\xC3" "A",
        "\xE0\xA0" "A",
        "\xC3",
        "\xE0\xA0",
        "\xF0\x9F\x98",
        "\xC0\xAF",
        "\xE0\x80\xAF",
        "\xF0\x80\x80\xAF",
        "\xED\xA0\x80",
        "\xED\xBF\xBF",
        "\xF4\x90\x80\x80",
    };
    for (size_t i = 0; i < std::size(samples); ++i) {
        EXPECT_FALSE(UTF::decode_utf8(samples[i]).has_value()) << "sample " << i;
    }
}

TEST(UTF, utf8_round_trips_boundary_values)
{
    const char32_t values[] = {
        0x7Fu, 0x80u, 0x7FFu, 0x800u, 0xD7FFu, 0xE000u, 0xFFFFu, 0x10000u, 0x10FFFFu};
    for (const char32_t value : values) {
        const std::u32string text(1, value);
        const std::string encoded = UTF::encode_utf8(text);
        const uint32_t code = value;
        EXPECT_EQ(encoded.size(), UTF::utf8_length(text)) << "value " << code;
        EXPECT_EQ(UTF::decode_utf8(encoded), text) << "value " << code;
    }
    EXPECT_EQ(UTF::encode_utf8(U"\U0010FFFF"), "\xF4\x8F\xBF\xBF");
}

TEST(UTF, decode_utf16_accepts_valid_text)
{
    const char16_t pair[] = {0xD83D, 0xDE00};
    const char16_t max[] = {0xDBFF, 0xDFFF};
    EXPECT_EQ(UTF::decode_utf16(u""), std::u32string());
    EXPECT_EQ(UTF::decode_utf16(u"Aé한"), U"Aé한");
    EXPECT_EQ(UTF::decode_utf16(std::u16string_view(pair, 2)), U"\U0001F600");
    EXPECT_EQ(UTF::decode_utf16(std::u16string_view(max, 2)), U"\U0010FFFF");
    EXPECT_EQ(UTF::encode_utf16(U"\U0001F600"), std::u16string(pair, 2));
    EXPECT_EQ(UTF::encode_utf16(U"\U0010FFFF"), std::u16string(max, 2));
}

TEST(UTF, decode_utf16_rejects_unpaired_surrogates)
{
    const std::u16string samples[] = {
        std::u16string{char16_t{0xD83D}},
        std::u16string{char16_t{0xD83D}, u'A'},
        std::u16string{char16_t{0xD83D}, char16_t{0xD83D}},
        std::u16string{char16_t{0xDE00}},
        std::u16string{u'A', char16_t{0xDE00}, u'B'},
    };
    for (size_t i = 0; i < std::size(samples); ++i) {
        EXPECT_FALSE(UTF::decode_utf16(samples[i]).has_value()) << "sample " << i;
    }
}

TEST(UTF, encode_rejects_invalid_code_points)
{
    const std::u32string surrogate(1, char32_t{0xD800});
    const std::u32string too_large(1, char32_t{0x110000});
    EXPECT_THROW(UTF::encode_utf8(surrogate), std::invalid_argument);
    EXPECT_THROW(UTF::encode_utf8(too_large), std::invalid_argument);
    EXPECT_THROW(UTF::encode_utf16(surrogate), std::invalid_argument);
    EXPECT_THROW(UTF::encode_utf16(too_large), std::invalid_argument);
    EXPECT_THROW(UTF::path_from_u32(surrogate), std::invalid_argument);
}

TEST(UTF, is_valid_checks_code_points)
{
    EXPECT_TRUE(UTF::is_valid(U""));
    EXPECT_TRUE(UTF::is_valid(U"A퟿\U0010FFFF"));
    EXPECT_FALSE(UTF::is_valid(std::u32string(1, char32_t{0xDFFF})));
    EXPECT_FALSE(UTF::is_valid(std::u32string(1, char32_t{0x110000})));
}

TEST(UTF, lengths_count_encoded_units)
{
    const std::u32string text = U"Aé한\U0001F600";
    EXPECT_EQ(UTF::utf8_length(text), size_t{10});
    EXPECT_EQ(UTF::utf16_length(text), size_t{5});
    EXPECT_EQ(UTF::utf8_length(U""), size_t{0});
    EXPECT_EQ(UTF::utf16_length(U""), size_t{0});
}

TEST(UTF, path_round_trips)
{
    const std::string text = "folder/한글 😀.txt";
    const std::u32string name = U"한글 \U0001F600.txt";
    const std::optional<std::filesystem::path> path = UTF::path_from_utf8(text);
    ASSERT_TRUE(path.has_value());
    EXPECT_EQ(UTF::path_to_utf8(*path), text);
    EXPECT_EQ(path->filename(), UTF::path_from_u32(name));
    EXPECT_EQ(UTF::path_to_u32(path->filename()), name);
    EXPECT_FALSE(UTF::path_from_utf8("\xFF").has_value());
}

TEST(UTF, path_rejects_invalid_native_names)
{
#if defined(_WIN32)
    std::wstring native = L"a";
    native.push_back(static_cast<wchar_t>(0xD800));
    native.push_back(L'b');
#else
    const std::string native = "a\xFF" "b";
#endif
    const std::filesystem::path path(native);
    EXPECT_FALSE(UTF::path_to_u32(path).has_value());
    EXPECT_FALSE(UTF::path_to_utf8(path).has_value());
}
