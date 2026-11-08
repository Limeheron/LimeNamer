#include "span_finder_test.h"

TEST_F(SPAN_FINDER_TEST, finds_start_and_end)
{
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::start), U"abc"), points({0}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::end), U"abc"), points({3}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::start), U""), points({0}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::end), U""), points({0}));
}

TEST_F(SPAN_FINDER_TEST, finds_after_nth_character)
{
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::after_nth, 1), U"abcde"), points({1}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::after_nth, 5), U"abcde"), points({5}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::after_nth, 9), U"abcde"), points({5}));
}

TEST_F(SPAN_FINDER_TEST, finds_before_nth_character_from_end)
{
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::before_nth_from_end, 1), U"abcde"), points({4}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::before_nth_from_end, 5), U"abcde"), points({0}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::before_nth_from_end, 9), U"abcde"), points({0}));
}

TEST_F(SPAN_FINDER_TEST, counts_code_points)
{
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::after_nth, 2), U"한글이름"),
        points({2}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::after_nth, 1), U"\U0001F600ab"), points({1}));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::end), U"\U0001F600ab"), points({3}));
}

TEST_F(SPAN_FINDER_TEST, finds_text_occurrences)
{
    const std::u32string name = U"IMG_0001_IMG";
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::before_text, U"IMG"), name), points({0}));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::after_text, U"IMG"), name), points({3}));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::before_text, U"IMG", OCCURRENCE::last), name),
        points({9}));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::before_text, U"IMG", OCCURRENCE::all), name),
        points({0, 9}));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::after_text, U"IMG", OCCURRENCE::nth, 2), name),
        points({12}));
    EXPECT_EQ(finder_.find(
        around(POSITION_TYPE::before_text, U"IMG", OCCURRENCE::nth_from_end, 2), name),
        points({0}));
    EXPECT_TRUE(finder_.find(
        around(POSITION_TYPE::before_text, U"IMG", OCCURRENCE::nth, 3), name).empty());
    EXPECT_TRUE(finder_.find(
        around(POSITION_TYPE::before_text, U"IMG", OCCURRENCE::nth_from_end, 3), name).empty());
}

TEST_F(SPAN_FINDER_TEST, returns_nothing_without_a_match)
{
    EXPECT_TRUE(finder_.find(around(POSITION_TYPE::before_text, U"x"), U"abc").empty());
    EXPECT_TRUE(finder_.find(around(POSITION_TYPE::before_text, U""), U"abc").empty());
    EXPECT_TRUE(finder_.find(around(POSITION_TYPE::after_text, U"abcd"), U"abc").empty());
    EXPECT_TRUE(finder_.find(around(POSITION_TYPE::after_text, U"a"), U"").empty());
}

TEST_F(SPAN_FINDER_TEST, ignores_case_unless_asked)
{
    const std::u32string name = U"img_IMG_Img";
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::before_text, U"img", OCCURRENCE::all), name),
        points({0, 4, 8}));
    EXPECT_EQ(finder_.find(
        around(POSITION_TYPE::before_text, U"img", OCCURRENCE::all, 1, true), name),
        points({0}));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::before_text, U"é", OCCURRENCE::all),
        U"Ésta éste"), points({0, 5}));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::after_text, U"ж", OCCURRENCE::all),
        U"Жж"), points({1, 2}));
}

TEST_F(SPAN_FINDER_TEST, matches_do_not_overlap)
{
    const std::vector<SPAN> expected = {{0, 2}, {2, 4}};
    EXPECT_EQ(finder_.matches(U"aaaaa", U"aa", true), expected);
    EXPECT_EQ(finder_.matches(U"AaAaA", U"aa", false), expected);
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::before_text, U"aa", OCCURRENCE::last), U"aaaaa"),
        points({2}));
}

TEST_F(SPAN_FINDER_TEST, finds_all)
{
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::all), U"abc"), span(0, 3));
    EXPECT_EQ(finder_.find(at(POSITION_TYPE::all), U""), span(0, 0));
}

TEST_F(SPAN_FINDER_TEST, finds_first_and_last_characters)
{
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::first_n, 2), U"abcde"), span(0, 2));
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::first_n, 9), U"abcde"), span(0, 5));
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::last_n, 2), U"abcde"), span(3, 5));
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::last_n, 9), U"abcde"), span(0, 5));
    EXPECT_TRUE(finder_.find(range(POSITION_TYPE::first_n, 2), U"").empty());
    EXPECT_TRUE(finder_.find(range(POSITION_TYPE::last_n, 2), U"").empty());
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::first_n, 1), U"\U0001F600ab"), span(0, 1));
}

TEST_F(SPAN_FINDER_TEST, finds_characters_from_nth)
{
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::from_nth, 2, 2), U"abcde"), span(1, 3));
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::from_nth, 4, 9), U"abcde"), span(3, 5));
    EXPECT_EQ(finder_.find(range(POSITION_TYPE::from_nth, 5, 1), U"abcde"), span(4, 5));
    EXPECT_TRUE(finder_.find(range(POSITION_TYPE::from_nth, 6, 1), U"abcde").empty());
    EXPECT_TRUE(finder_.find(range(POSITION_TYPE::from_nth, 1, 1), U"").empty());
}

TEST_F(SPAN_FINDER_TEST, finds_text_ranges)
{
    const std::u32string name = U"IMG_0001_img";
    const std::vector<SPAN> both = {{0, 3}, {9, 12}};
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::text, U"IMG", OCCURRENCE::all), name), both);
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::text, U"IMG"), name), span(0, 3));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::text, U"IMG", OCCURRENCE::last), name),
        span(9, 12));
    EXPECT_EQ(finder_.find(around(POSITION_TYPE::text, U"IMG", OCCURRENCE::all, 1, true), name),
        span(0, 3));
    EXPECT_TRUE(finder_.find(around(POSITION_TYPE::text, U"x"), name).empty());
}

TEST_F(SPAN_FINDER_TEST, finds_text_between_delimiters)
{
    const std::u32string name = U"photo[2024]_[final]";
    EXPECT_EQ(finder_.find(between(U"[", U"]"), name), span(6, 10));
    EXPECT_EQ(finder_.find(including(between(U"[", U"]")), name), span(5, 11));
    EXPECT_EQ(finder_.find(between(U"[", U"]", OCCURRENCE::last), name), span(13, 18));
    EXPECT_EQ(finder_.find(including(between(U"[", U"]", OCCURRENCE::last)), name),
        span(12, 19));
    EXPECT_EQ(finder_.find(between(U"[", U"]", OCCURRENCE::nth, 2), name), span(13, 18));
    EXPECT_EQ(finder_.find(between(U"[", U"]", OCCURRENCE::nth_from_end, 2), name),
        span(6, 10));
    EXPECT_TRUE(finder_.find(between(U"[", U"]", OCCURRENCE::nth, 3), name).empty());
}

TEST_F(SPAN_FINDER_TEST, finds_text_between_ends)
{
    const std::u32string name = U"photo[2024]_[final]";
    EXPECT_EQ(finder_.find(between(U"", U"_"), name), span(0, 11));
    EXPECT_EQ(finder_.find(including(between(U"", U"_")), name), span(0, 12));
    EXPECT_EQ(finder_.find(between(U"", U"]", OCCURRENCE::last), name), span(0, 18));
    EXPECT_EQ(finder_.find(between(U"_", U""), name), span(12, 19));
    EXPECT_EQ(finder_.find(including(between(U"_", U"")), name), span(11, 19));
    EXPECT_EQ(finder_.find(between(U"", U""), name), span(0, 19));
}

TEST_F(SPAN_FINDER_TEST, needs_both_delimiters)
{
    EXPECT_TRUE(finder_.find(between(U"[", U"]"), U"a[b").empty());
    EXPECT_TRUE(finder_.find(between(U"[", U"]"), U"a]b").empty());
    EXPECT_TRUE(finder_.find(between(U"[", U"]"), U"a]b[c").empty());
    EXPECT_TRUE(finder_.find(between(U"", U"]"), U"abc").empty());
    EXPECT_TRUE(finder_.find(between(U"[", U""), U"abc").empty());
    EXPECT_EQ(finder_.find(between(U"[", U"]"), U"x[]y"), span(2, 2));
}

TEST_F(SPAN_FINDER_TEST, matches_delimiters_by_case_setting)
{
    EXPECT_EQ(finder_.find(between(U"a", U"a"), U"AbcA"), span(1, 3));
    POSITION exact = between(U"a", U"a");
    exact.case_sensitive = true;
    EXPECT_TRUE(finder_.find(exact, U"AbcA").empty());
}
