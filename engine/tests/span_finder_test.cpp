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
