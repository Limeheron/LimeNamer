#include "name_checker_test.h"

TEST_F(NAME_CHECKER_TEST, accepts_ordinary_names)
{
    for (const std::u32string_view name : {U"photo.jpg", U"\uD55C\uAE00 \uD30C\uC77C.txt",
             U"a b-c_d (1).tar.gz", U"\U0001F600.png", U"CONSOLE.txt", U"COM10", U"xCON"}) {
        EXPECT_EQ(on_windows(name), result(0, 0)) << UTF::encode_utf8(name);
        EXPECT_EQ(on_mac(name), result(0, 0)) << UTF::encode_utf8(name);
    }
}

TEST_F(NAME_CHECKER_TEST, rejects_empty_and_dot_names)
{
    for (const std::u32string_view name : {U"", U"   ", U"\u3000"}) {
        EXPECT_TRUE(on_windows(name).errors & NAME_ISSUE::EMPTY) << UTF::encode_utf8(name);
        EXPECT_TRUE(on_mac(name).errors & NAME_ISSUE::EMPTY) << UTF::encode_utf8(name);
    }
    for (const std::u32string_view name : {U".", U".."}) {
        EXPECT_TRUE(on_windows(name).errors & NAME_ISSUE::DOT_NAME);
        EXPECT_TRUE(on_mac(name).errors & NAME_ISSUE::DOT_NAME);
        EXPECT_FALSE(on_mac(name).warnings & NAME_ISSUE::HIDDEN);
    }
}

TEST_F(NAME_CHECKER_TEST, applies_windows_character_rules)
{
    for (const std::u32string_view name : {U"a\\b", U"a/b", U"a:b", U"a*b", U"a?b", U"a\"b",
             U"a<b", U"a>b", U"a|b", U"a\u0001b"}) {
        EXPECT_TRUE(on_windows(name).errors & NAME_ISSUE::WIN_FORBIDDEN_CHAR)
            << UTF::encode_utf8(name);
    }
    EXPECT_EQ(on_mac(U"a?b"), result(0, NAME_ISSUE::WIN_FORBIDDEN_CHAR));
}

TEST_F(NAME_CHECKER_TEST, applies_windows_reserved_names)
{
    for (const std::u32string_view name : {U"CON", U"con.txt", U"NUL.tar.gz", U"Aux ",
             U"PRN.doc", U"COM1", U"com9.log", U"COM0", U"LPT5", U"LPT\u00B9", U"COM\u00B3.txt"}) {
        EXPECT_TRUE(on_windows(name).errors & NAME_ISSUE::WIN_RESERVED) << UTF::encode_utf8(name);
        EXPECT_TRUE(on_mac(name).warnings & NAME_ISSUE::WIN_RESERVED) << UTF::encode_utf8(name);
    }
    for (const std::u32string_view name : {U"COM", U"COM10", U"CONSOLE", U" CON", U"LPTX"}) {
        EXPECT_FALSE(on_windows(name).errors & NAME_ISSUE::WIN_RESERVED)
            << UTF::encode_utf8(name);
    }
}

TEST_F(NAME_CHECKER_TEST, applies_windows_trailing_rule)
{
    EXPECT_EQ(on_windows(U"name."), result(NAME_ISSUE::WIN_TRAILING, 0));
    EXPECT_EQ(on_windows(U"name "), result(NAME_ISSUE::WIN_TRAILING, 0));
    EXPECT_EQ(on_mac(U"name."), result(0, NAME_ISSUE::WIN_TRAILING));
}

TEST_F(NAME_CHECKER_TEST, applies_mac_character_rules)
{
    EXPECT_TRUE(on_mac(U"a/b").errors & NAME_ISSUE::MAC_FORBIDDEN_CHAR);
    EXPECT_TRUE(on_mac(U"a:b").errors & NAME_ISSUE::MAC_FORBIDDEN_CHAR);
    EXPECT_TRUE(on_mac(std::u32string(U"a\0b", 3)).errors & NAME_ISSUE::MAC_FORBIDDEN_CHAR);
    EXPECT_TRUE(on_windows(U"a:b").warnings & NAME_ISSUE::MAC_FORBIDDEN_CHAR);
}

TEST_F(NAME_CHECKER_TEST, counts_name_length_per_system)
{
    const std::u32string ascii_ok(255, U'a');
    const std::u32string ascii_long(256, U'a');
    const std::u32string korean(100, U'\uD55C');
    const std::u32string emoji(128, U'\U0001F600');
    EXPECT_EQ(windows_.check(ascii_ok, U"a", 0), result(0, 0));
    EXPECT_EQ(windows_.check(ascii_long, U"a", 0),
        result(NAME_ISSUE::WIN_NAME_TOO_LONG, NAME_ISSUE::MAC_NAME_TOO_LONG));
    EXPECT_EQ(windows_.check(korean, U"a", 0), result(0, NAME_ISSUE::MAC_NAME_TOO_LONG));
    EXPECT_EQ(mac_.check(korean, U"a", 0), result(NAME_ISSUE::MAC_NAME_TOO_LONG, 0));
    EXPECT_EQ(mac_.check(ascii_long, U"a", 0),
        result(NAME_ISSUE::MAC_NAME_TOO_LONG, NAME_ISSUE::WIN_NAME_TOO_LONG));
    EXPECT_EQ(mac_.check(ascii_ok, U"a", 0), result(0, 0));
    EXPECT_TRUE(windows_.check(emoji, U"a", 0).errors & NAME_ISSUE::WIN_NAME_TOO_LONG);
}

TEST_F(NAME_CHECKER_TEST, checks_windows_path_length)
{
    const std::u32string name(10, U'a');
    EXPECT_EQ(windows_.check(name, U"a", 250), result(NAME_ISSUE::WIN_PATH_TOO_LONG, 0));
    EXPECT_EQ(windows_.check(name, U"a", 249), result(0, 0));
    EXPECT_EQ(windows_.check(name, std::u32string(12, U'b'), 250), result(0, 0));
    EXPECT_EQ(windows_.check(name, std::u32string(10, U'b'), 250), result(0, 0));
    EXPECT_EQ(mac_.check(name, U"a", 250), result(0, 0));
}

TEST_F(NAME_CHECKER_TEST, counts_folder_units)
{
    EXPECT_EQ(NAME_CHECKER::folder_units(U"C:\\a"), size_t{5});
    EXPECT_EQ(NAME_CHECKER::folder_units(U"C:\\"), size_t{3});
    EXPECT_EQ(NAME_CHECKER::folder_units(U"/"), size_t{1});
    EXPECT_EQ(NAME_CHECKER::folder_units(U"/fake/\U0001F600"), size_t{9});
}

TEST_F(NAME_CHECKER_TEST, warns_about_hidden_names)
{
    EXPECT_EQ(on_windows(U".config"), result(0, NAME_ISSUE::HIDDEN));
    EXPECT_EQ(on_mac(U".config"), result(0, NAME_ISSUE::HIDDEN));
}

TEST_F(NAME_CHECKER_TEST, warns_only_about_new_conditions)
{
    EXPECT_EQ(on_mac(U".new", U".old"), result(0, 0));
    EXPECT_EQ(on_mac(U"c?d", U"a?b"), result(0, 0));
    EXPECT_EQ(on_mac(U"c?d.", U"a?b"), result(0, NAME_ISSUE::WIN_TRAILING));
    EXPECT_EQ(on_windows(U"\uD55C" + std::u32string(99, U'\uD55C'), std::u32string(90, U'\uD55C')),
        result(0, 0));
}
