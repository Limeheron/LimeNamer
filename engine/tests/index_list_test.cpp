#include "index_list_test.h"

namespace
{
std::vector<uint32_t> indices_of(std::u32string_view text, uint32_t count)
{
    const INDEX_LIST_RESULT result = INDEX_LIST::parse(text, count);
    EXPECT_EQ(result.error, INDEX_ERROR::none) << UTF::encode_utf8(text);
    return result.indices;
}
}

TEST(INDEX_LIST, parses_numbers_and_ranges)
{
    EXPECT_EQ(indices_of(U"3", 10), std::vector<uint32_t>({3}));
    EXPECT_EQ(indices_of(U"1,3,5", 10), std::vector<uint32_t>({1, 3, 5}));
    EXPECT_EQ(indices_of(U"2-4", 10), std::vector<uint32_t>({2, 3, 4}));
    EXPECT_EQ(indices_of(U"2~4", 10), std::vector<uint32_t>({2, 3, 4}));
    EXPECT_EQ(indices_of(U"3-5, 8", 10), std::vector<uint32_t>({3, 4, 5, 8}));
    EXPECT_EQ(indices_of(U"3-500, 600", 600).size(), size_t{499});
}

TEST(INDEX_LIST, accepts_spaces_reversed_ranges_and_duplicates)
{
    EXPECT_EQ(indices_of(U"  1 ,  4 - 2 ", 10), std::vector<uint32_t>({1, 2, 3, 4}));
    EXPECT_EQ(indices_of(U"5-3", 10), std::vector<uint32_t>({3, 4, 5}));
    EXPECT_EQ(indices_of(U"2, 1-3, 2", 10), std::vector<uint32_t>({1, 2, 3}));
    EXPECT_EQ(indices_of(U"　1　", 10), std::vector<uint32_t>({1}));
    EXPECT_EQ(indices_of(U"007", 10), std::vector<uint32_t>({7}));
}

TEST(INDEX_LIST, treats_empty_text_as_no_items)
{
    EXPECT_TRUE(indices_of(U"", 10).empty());
    EXPECT_TRUE(indices_of(U"   ", 10).empty());
}

TEST(INDEX_LIST, rejects_bad_syntax)
{
    const std::u32string_view samples[] = {
        U"a", U"1,", U",1", U"1,,2", U"-5", U"5-", U"1-2-3", U"3 5", U"1.5", U"1;2", U"１",
    };
    for (const std::u32string_view sample : samples) {
        EXPECT_EQ(INDEX_LIST::parse(sample, 10).error, INDEX_ERROR::syntax)
            << UTF::encode_utf8(sample);
    }
}

TEST(INDEX_LIST, rejects_numbers_outside_the_list)
{
    const std::u32string_view samples[] = {U"0", U"11", U"0-3", U"9-11", U"99999999999999"};
    for (const std::u32string_view sample : samples) {
        const INDEX_LIST_RESULT result = INDEX_LIST::parse(sample, 10);
        EXPECT_EQ(result.error, INDEX_ERROR::out_of_range) << UTF::encode_utf8(sample);
        EXPECT_TRUE(result.indices.empty());
    }
    EXPECT_EQ(INDEX_LIST::parse(U"1", 0).error, INDEX_ERROR::out_of_range);
}

TEST(INDEX_LIST, formats_indices)
{
    EXPECT_EQ(INDEX_LIST::format({}), U"");
    EXPECT_EQ(INDEX_LIST::format({3}), U"3");
    EXPECT_EQ(INDEX_LIST::format({1, 2}), U"1, 2");
    EXPECT_EQ(INDEX_LIST::format({1, 2, 3}), U"1-3");
    EXPECT_EQ(INDEX_LIST::format({8, 3, 4, 5, 600, 3}), U"3-5, 8, 600");
    EXPECT_EQ(INDEX_LIST::format({1, 2, 4, 5, 6, 9}), U"1, 2, 4-6, 9");
}

TEST(INDEX_LIST, round_trips_formatted_text)
{
    const std::vector<uint32_t> indices = {1, 2, 4, 5, 6, 9, 10, 11, 20};
    EXPECT_EQ(indices_of(INDEX_LIST::format(indices), 20), indices);
}
