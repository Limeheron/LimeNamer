#include "c_api_index_test.h"

namespace
{
std::vector<uint32_t> indices_of(const LN_INDEX_LIST &list)
{
    return std::vector<uint32_t>(list.indices, list.indices + list.count);
}

std::string format(const std::vector<uint32_t> &indices)
{
    char *text = nullptr;
    EXPECT_EQ(ln_index_list_format(indices.data(), indices.size(), &text), ln_result_ok);
    const std::string result = text == nullptr ? std::string() : std::string(text);
    ln_string_free(text);
    return result;
}
}

TEST(C_API_INDEX, parses_index_lists)
{
    LN_INDEX_LIST *list = nullptr;
    ASSERT_EQ(ln_index_list_parse("3-5, 1", 10, &list), ln_result_ok);
    EXPECT_EQ(indices_of(*list), std::vector<uint32_t>({1, 3, 4, 5}));
    EXPECT_EQ(list->error, ln_index_error_none);
    ln_index_list_free(list);

    ASSERT_EQ(ln_index_list_parse("8~6\xe3\x80\x80,2", 10, &list), ln_result_ok);
    EXPECT_EQ(indices_of(*list), std::vector<uint32_t>({2, 6, 7, 8}));
    ln_index_list_free(list);

    ASSERT_EQ(ln_index_list_parse("", 10, &list), ln_result_ok);
    EXPECT_EQ(list->count, size_t{0});
    EXPECT_EQ(list->indices, nullptr);
    EXPECT_EQ(list->error, ln_index_error_none);
    ln_index_list_free(list);
}

TEST(C_API_INDEX, reports_index_list_errors)
{
    LN_INDEX_LIST *list = nullptr;
    ASSERT_EQ(ln_index_list_parse("1,,2", 10, &list), ln_result_ok);
    EXPECT_EQ(list->error, ln_index_error_syntax);
    EXPECT_EQ(list->count, size_t{0});
    ln_index_list_free(list);

    ASSERT_EQ(ln_index_list_parse("11", 10, &list), ln_result_ok);
    EXPECT_EQ(list->error, ln_index_error_out_of_range);
    ln_index_list_free(list);

    EXPECT_EQ(ln_index_list_parse("\xff", 10, &list), ln_result_invalid_argument);
    EXPECT_EQ(list, nullptr);
    EXPECT_EQ(ln_index_list_parse(nullptr, 10, &list), ln_result_invalid_argument);
    EXPECT_EQ(ln_index_list_parse("1", 10, nullptr), ln_result_invalid_argument);
}

TEST(C_API_INDEX, formats_index_lists)
{
    EXPECT_EQ(format({9, 1, 2, 4, 5, 6, 5}), "1, 2, 4-6, 9");
    EXPECT_EQ(format({}), "");

    char *text = nullptr;
    const uint32_t zero[] = {0, 1};
    EXPECT_EQ(ln_index_list_format(zero, 2, &text), ln_result_invalid_argument);
    EXPECT_EQ(text, nullptr);
    EXPECT_EQ(ln_index_list_format(nullptr, 1, &text), ln_result_invalid_argument);
    EXPECT_EQ(ln_index_list_format(nullptr, 0, nullptr), ln_result_invalid_argument);
    ASSERT_EQ(ln_index_list_format(nullptr, 0, &text), ln_result_ok);
    EXPECT_EQ(std::string(text), "");
    ln_string_free(text);
}
