#include "natural_compare_test.h"

TEST_F(NATURAL_COMPARE_TEST, sorts_numbers_by_value)
{
    const std::vector<std::u32string> expected = {
        U"file1.txt", U"file2.txt", U"file10.txt", U"file100.txt"};
    EXPECT_EQ(sorted({U"file10.txt", U"file2.txt", U"file100.txt", U"file1.txt"}, true),
        expected);
}

TEST_F(NATURAL_COMPARE_TEST, sorts_characters_in_order)
{
    const std::vector<std::u32string> expected = {
        U"file1.txt", U"file10.txt", U"file100.txt", U"file2.txt"};
    EXPECT_EQ(sorted({U"file10.txt", U"file2.txt", U"file100.txt", U"file1.txt"}, false),
        expected);
}

TEST_F(NATURAL_COMPARE_TEST, compares_every_number_in_the_name)
{
    EXPECT_LT(compare_.compare(U"a2b9", U"a2b10", true), 0);
    EXPECT_LT(compare_.compare(U"v1.9.2", U"v1.10.0", true), 0);
    EXPECT_GT(compare_.compare(U"a12345678901234567890", U"a9", true), 0);
    EXPECT_LT(compare_.compare(U"1a", U"a1", true), 0);
}

TEST_F(NATURAL_COMPARE_TEST, puts_fewer_leading_zeros_first)
{
    const std::vector<std::u32string> expected = {U"file1", U"file01", U"file001", U"file2"};
    EXPECT_EQ(sorted({U"file001", U"file2", U"file01", U"file1"}, true), expected);
    EXPECT_LT(compare_.compare(U"0", U"00", true), 0);
}

TEST_F(NATURAL_COMPARE_TEST, ignores_case_then_breaks_ties)
{
    const std::vector<std::u32string> expected = {U"Apple", U"apple", U"banana", U"Cherry"};
    EXPECT_EQ(sorted({U"Cherry", U"banana", U"apple", U"Apple"}, true), expected);
    EXPECT_EQ(sorted({U"Cherry", U"banana", U"apple", U"Apple"}, false), expected);
    EXPECT_EQ(compare_.compare(U"same", U"same", true), 0);
}

TEST_F(NATURAL_COMPARE_TEST, sorts_other_scripts)
{
    const std::vector<std::u32string> expected = {
        U"ÉTOILE", U"été", U"가2", U"가10", U"나"};
    EXPECT_EQ(sorted({U"ÉTOILE", U"나", U"가10", U"été", U"가2"}, true),
        expected);
}

TEST_F(NATURAL_COMPARE_TEST, handles_empty_names)
{
    EXPECT_EQ(compare_.compare(U"", U"", true), 0);
    EXPECT_LT(compare_.compare(U"", U"a", true), 0);
    EXPECT_GT(compare_.compare(U"a", U"", false), 0);
}

TEST_F(NATURAL_COMPARE_TEST, makes_lowercase_keys)
{
    EXPECT_EQ(compare_.key(U"File_A.TXT"), U"file_a.txt");
    EXPECT_EQ(compare_.key(U"ÉTÉ"), U"été");
    EXPECT_LT(NATURAL_COMPARE::compare_keys(U"a_b", U"ab", true), 0);
}
