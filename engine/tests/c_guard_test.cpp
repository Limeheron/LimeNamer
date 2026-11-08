#include "c_guard_test.h"

TEST_F(C_GUARD_TEST, maps_exceptions_to_results)
{
    EXPECT_EQ(C_GUARD::run([]() -> LN_RESULT { return ln_result_not_found; }),
        ln_result_not_found);
    EXPECT_EQ(C_GUARD::run([]() -> LN_RESULT { throw MOD_JSON_ERROR("bad"); }),
        ln_result_invalid_json);
    EXPECT_EQ(C_GUARD::run([]() -> LN_RESULT { throw SESSION_BUSY(); }), ln_result_busy);
    EXPECT_EQ(C_GUARD::run([]() -> LN_RESULT { throw std::bad_alloc(); }),
        ln_result_out_of_memory);
    EXPECT_EQ(C_GUARD::run([]() -> LN_RESULT { throw std::invalid_argument("bad"); }),
        ln_result_invalid_argument);
    EXPECT_EQ(C_GUARD::run([]() -> LN_RESULT { throw std::runtime_error("bad"); }),
        ln_result_internal);
    EXPECT_EQ(C_GUARD::run([]() -> LN_RESULT { throw 1; }), ln_result_internal);
}

TEST_F(C_GUARD_TEST, passes_the_session)
{
    const SESSION *seen = nullptr;
    EXPECT_EQ(C_GUARD::run(&session_, [&](SESSION &session) -> LN_RESULT {
        seen = &session;
        return ln_result_ok;
    }), ln_result_ok);
    EXPECT_EQ(seen, &session_.session);
    EXPECT_FALSE(session_.busy.load());
}

TEST_F(C_GUARD_TEST, rejects_a_missing_session)
{
    bool called = false;
    EXPECT_EQ(C_GUARD::run(nullptr, [&](SESSION &) -> LN_RESULT {
        called = true;
        return ln_result_ok;
    }), ln_result_invalid_argument);
    EXPECT_FALSE(called);
}

TEST_F(C_GUARD_TEST, rejects_calls_while_another_runs)
{
    LN_RESULT inner = ln_result_ok;
    bool inner_called = false;
    EXPECT_EQ(C_GUARD::run(&session_, [&](SESSION &) -> LN_RESULT {
        inner = C_GUARD::run(&session_, [&](SESSION &) -> LN_RESULT {
            inner_called = true;
            return ln_result_ok;
        });
        return ln_result_ok;
    }), ln_result_ok);
    EXPECT_EQ(inner, ln_result_busy);
    EXPECT_FALSE(inner_called);
}

TEST_F(C_GUARD_TEST, frees_the_session_after_a_failure)
{
    EXPECT_EQ(C_GUARD::run(&session_, [](SESSION &) -> LN_RESULT {
        throw std::runtime_error("bad");
    }), ln_result_internal);
    EXPECT_FALSE(session_.busy.load());
    EXPECT_EQ(C_GUARD::run(&session_, [](SESSION &) -> LN_RESULT { return ln_result_ok; }),
        ln_result_ok);
}
