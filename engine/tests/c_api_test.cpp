#include "c_api_test.h"

TEST(C_API, version_matches_token)
{
    const std::string prefix = "LIMENAMER_VERSION=";
    EXPECT_EQ(prefix + ln_version(), SESSION::VERSION_TOKEN);
}

TEST(C_API, result_codes_are_stable)
{
    EXPECT_EQ(ln_result_ok, 0);
    EXPECT_EQ(ln_result_invalid_argument, 1);
    EXPECT_EQ(ln_result_invalid_json, 2);
    EXPECT_EQ(ln_result_not_found, 3);
    EXPECT_EQ(ln_result_busy, 4);
    EXPECT_EQ(ln_result_not_ready, 5);
    EXPECT_EQ(ln_result_io, 6);
    EXPECT_EQ(ln_result_out_of_memory, 7);
    EXPECT_EQ(ln_result_internal, 8);
    EXPECT_EQ(sizeof(LN_RESULT), sizeof(int32_t));
}

TEST(C_API, session_create_and_destroy)
{
    LN_SESSION *session = nullptr;
    ASSERT_EQ(ln_session_create(&session), ln_result_ok);
    EXPECT_NE(session, nullptr);
    ln_session_destroy(session);
}

TEST(C_API, sessions_are_independent)
{
    LN_SESSION *first = nullptr;
    LN_SESSION *second = nullptr;
    ASSERT_EQ(ln_session_create(&first), ln_result_ok);
    ASSERT_EQ(ln_session_create(&second), ln_result_ok);
    EXPECT_NE(first, second);
    ln_session_destroy(first);
    ln_session_destroy(second);
}

TEST(C_API, session_create_rejects_null_output)
{
    EXPECT_EQ(ln_session_create(nullptr), ln_result_invalid_argument);
}

TEST(C_API, session_destroy_accepts_null)
{
    ln_session_destroy(nullptr);
}

TEST(C_API, free_functions_accept_null)
{
    ln_string_free(nullptr);
    ln_problems_free(nullptr);
    ln_example_free(nullptr);
    ln_preview_free(nullptr);
    ln_report_free(nullptr);
    ln_index_list_free(nullptr);
}
