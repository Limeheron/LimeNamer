#include "c_api_mods_test.h"

namespace
{
const char *const PREFIX =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a_"}]})";

const char *const UPPER =
    R"({"kind":"format","position":{"type":"all"},"format":{"type":"upper"}})";

const char *const SAME =
    R"({"kind":"replace","position":{"type":"all"},"value":[{"text":"same"}]})";

bool contains(const std::string &text, const std::string &part)
{
    return text.find(part) != std::string::npos;
}
}

TEST_F(C_API_TEST, adds_updates_and_lists_mods)
{
    LN_ADD_RESULT added{};
    add({make_file("x.txt")}, ln_add_mode_as_items, added);
    uint64_t id = 0;
    ASSERT_EQ(ln_mod_add(session_, PREFIX, &id), ln_result_ok);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(new_names(), std::vector<std::string>({"a_x.txt"}));
    const std::string json = mods_json();
    EXPECT_TRUE(contains(json, R"("id":1)")) << json;
    EXPECT_TRUE(contains(json, R"("enabled":true)")) << json;
    EXPECT_TRUE(contains(json, R"("kind":"insert")")) << json;

    ASSERT_EQ(ln_mod_update(session_, id,
        R"({"kind":"insert","position":{"type":"end"},"value":[{"text":"_b"}]})"), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"x_b.txt"}));
    EXPECT_EQ(ln_mod_update(session_, id, UPPER), ln_result_invalid_argument);
    EXPECT_EQ(ln_mod_update(session_, 99, PREFIX), ln_result_not_found);
    EXPECT_EQ(ln_mod_update(session_, id, nullptr), ln_result_invalid_argument);

    EXPECT_EQ(ln_mod_add(session_, "{", &id), ln_result_invalid_json);
    EXPECT_EQ(ln_mod_add(session_, R"({"position":{"type":"start"}})", &id),
        ln_result_invalid_json);
    EXPECT_EQ(ln_mod_add(session_, nullptr, &id), ln_result_invalid_argument);
    EXPECT_EQ(ln_mod_add(session_, PREFIX, nullptr), ln_result_invalid_argument);
    EXPECT_EQ(ln_mods_get(session_, nullptr), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, removes_moves_and_toggles_mods)
{
    LN_ADD_RESULT added{};
    add({make_file("x.txt")}, ln_add_mode_as_items, added);
    uint64_t prefix = 0;
    uint64_t upper = 0;
    ln_mod_add(session_, PREFIX, &prefix);
    ln_mod_add(session_, UPPER, &upper);
    EXPECT_EQ(new_names(), std::vector<std::string>({"A_X.txt"}));

    uint8_t changed = 9;
    ASSERT_EQ(ln_mod_move(session_, upper, prefix, 0, 1, &changed), ln_result_ok);
    EXPECT_EQ(changed, 1);
    EXPECT_EQ(new_names(), std::vector<std::string>({"A_X.txt"}));
    ASSERT_EQ(ln_mod_move(session_, upper, prefix, 0, 0, &changed), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"a_X.txt"}));
    EXPECT_EQ(ln_mod_move(session_, upper, 99, 0, 0, &changed), ln_result_not_found);
    EXPECT_EQ(ln_mod_move(session_, upper, prefix, 2, 0, &changed), ln_result_invalid_argument);
    EXPECT_EQ(ln_mod_move(session_, upper, prefix, 0, 0, nullptr), ln_result_invalid_argument);

    ASSERT_EQ(ln_mod_set_enabled(session_, upper, 0), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"a_x.txt"}));
    EXPECT_TRUE(contains(mods_json(), R"("enabled":false)"));
    EXPECT_EQ(ln_mod_set_enabled(session_, 99, 0), ln_result_not_found);
    EXPECT_EQ(ln_mod_set_enabled(session_, upper, 2), ln_result_invalid_argument);
    ASSERT_EQ(ln_mods_set_all_enabled(session_, 0), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"x.txt"}));
    ASSERT_EQ(ln_mods_set_all_enabled(session_, 1), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"a_X.txt"}));
    EXPECT_EQ(ln_mods_set_all_enabled(session_, 2), ln_result_invalid_argument);

    ASSERT_EQ(ln_mod_remove(session_, prefix), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"X.txt"}));
    EXPECT_EQ(ln_mod_remove(session_, prefix), ln_result_not_found);
}

TEST_F(C_API_TEST, edits_names_directly)
{
    LN_ADD_RESULT added{};
    add({make_file("x.txt"), make_file("y.txt")}, ln_add_mode_as_items, added);
    const std::vector<uint64_t> items = ids();
    ASSERT_EQ(ln_mod_direct_edit(session_, items[1], "\xea\xb0\x80.txt"), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"x.txt", "\xea\xb0\x80.txt"}));
    LN_PREVIEW *preview = nullptr;
    ASSERT_EQ(ln_preview(session_, &preview), ln_result_ok);
    EXPECT_EQ(preview->rows[0].direct, 0);
    EXPECT_EQ(preview->rows[1].direct, 1);
    ln_preview_free(preview);
    EXPECT_TRUE(contains(mods_json(), R"("direct":true)"));
    EXPECT_EQ(ln_mod_direct_edit(session_, 99, "z.txt"), ln_result_not_found);
    EXPECT_EQ(ln_mod_direct_edit(session_, items[0], "\xff"), ln_result_invalid_argument);
    EXPECT_EQ(ln_mod_direct_edit(session_, items[0], nullptr), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, adds_only_regular_mods)
{
    uint64_t id = 0;
    ASSERT_EQ(ln_mod_add(session_, R"({"kind":"replace","position":{"type":"all"},
        "value":[{"text":"abc"}],"scope":"items","targets":[1],"direct":true})", &id),
        ln_result_ok);
    EXPECT_TRUE(contains(mods_json(), R"("direct":false)"));
}

TEST_F(C_API_TEST, checks_mods)
{
    LN_PROBLEMS *problems = nullptr;
    ASSERT_EQ(ln_mod_check(session_,
        R"({"kind":"replace","position":{"type":"text","text":""},"value":[{"text":""}]})",
        &problems), ln_result_ok);
    ASSERT_EQ(problems->count, size_t{2});
    EXPECT_EQ(problems->items[0].field, ln_mod_field_find_text);
    EXPECT_EQ(problems->items[1].field, ln_mod_field_value);
    EXPECT_EQ(problems->items[1].reason, ln_problem_reason_required);
    ln_problems_free(problems);

    ASSERT_EQ(ln_mod_check(session_, PREFIX, &problems), ln_result_ok);
    EXPECT_EQ(problems->count, size_t{0});
    EXPECT_EQ(problems->items, nullptr);
    ln_problems_free(problems);

    problems = reinterpret_cast<LN_PROBLEMS *>(&problems);
    EXPECT_EQ(ln_mod_check(session_, "[]", &problems), ln_result_invalid_json);
    EXPECT_EQ(problems, nullptr);
    EXPECT_EQ(ln_mod_check(session_, PREFIX, nullptr), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, finds_examples)
{
    LN_EXAMPLE *example = nullptr;
    ASSERT_EQ(ln_mod_example(session_, PREFIX, 0, &example), ln_result_ok);
    EXPECT_EQ(example->item_id, 0u);
    EXPECT_EQ(std::string(example->before), "IMG_0001.JPG");
    EXPECT_EQ(std::string(example->after), "a_IMG_0001.JPG");
    ln_example_free(example);

    LN_ADD_RESULT added{};
    add({make_file("x.txt")}, ln_add_mode_as_items, added);
    uint64_t upper = 0;
    ln_mod_add(session_, UPPER, &upper);
    ASSERT_EQ(ln_mod_example(session_, PREFIX, 0, &example), ln_result_ok);
    EXPECT_EQ(example->item_id, ids()[0]);
    EXPECT_EQ(example->index, 1u);
    EXPECT_EQ(std::string(example->before), "X.txt");
    EXPECT_EQ(std::string(example->after), "a_X.txt");
    EXPECT_EQ(example->prefix_len, 0u);
    EXPECT_EQ(example->suffix_len, 5u);
    EXPECT_EQ(example->state, ln_example_state_changed);
    ln_example_free(example);
    ASSERT_EQ(ln_mod_example(session_, PREFIX, upper, &example), ln_result_ok);
    EXPECT_EQ(std::string(example->before), "x.txt");
    ln_example_free(example);

    EXPECT_EQ(ln_mod_example(session_, PREFIX, 99, &example), ln_result_not_found);
    EXPECT_EQ(example, nullptr);
    EXPECT_EQ(ln_mod_example(session_, "{}", 0, &example), ln_result_invalid_json);
}

TEST_F(C_API_TEST, reads_and_sets_options)
{
    char *json = nullptr;
    ASSERT_EQ(ln_options_get(session_, &json), ln_result_ok);
    EXPECT_TRUE(contains(json, R"("enabled":false)")) << json;
    ln_string_free(json);

    LN_ADD_RESULT added{};
    add({make_file("x.txt"), make_file("y.txt")}, ln_add_mode_as_items, added);
    uint64_t id = 0;
    ln_mod_add(session_, SAME, &id);
    LN_PREVIEW *preview = nullptr;
    ASSERT_EQ(ln_preview(session_, &preview), ln_result_ok);
    EXPECT_EQ(preview->conflict, 2u);
    ln_preview_free(preview);

    ASSERT_EQ(ln_options_set(session_, R"({"duplicate":{"enabled":true}})"), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"same.txt", "same (1).txt"}));
    ASSERT_EQ(ln_options_get(session_, &json), ln_result_ok);
    EXPECT_TRUE(contains(json, R"("enabled":true)")) << json;
    ln_string_free(json);

    EXPECT_EQ(ln_options_set(session_, R"({"duplicate":{"format":"custom"}})"),
        ln_result_invalid_json);
    EXPECT_EQ(ln_options_set(session_, nullptr), ln_result_invalid_argument);
    EXPECT_EQ(ln_options_get(session_, nullptr), ln_result_invalid_argument);
}
