#include "c_api_history_test.h"

namespace
{
const char *const PREFIX =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a_"}]})";
}

TEST_F(C_API_TEST, undoes_and_redoes_changes)
{
    uint8_t can_undo = 9;
    uint8_t can_redo = 9;
    ASSERT_EQ(ln_history_state(session_, &can_undo, &can_redo), ln_result_ok);
    EXPECT_EQ(can_undo, 0);
    EXPECT_EQ(can_redo, 0);
    EXPECT_EQ(ln_undo(session_), ln_result_not_found);
    EXPECT_EQ(ln_redo(session_), ln_result_not_found);

    LN_ADD_RESULT added{};
    add({make_file("x.txt")}, ln_add_mode_as_items, added);
    uint64_t id = 0;
    ln_mod_add(session_, PREFIX, &id);
    EXPECT_EQ(new_names(), std::vector<std::string>({"a_x.txt"}));
    ASSERT_EQ(ln_undo(session_), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"x.txt"}));
    EXPECT_EQ(mods_json(), "[]");
    ASSERT_EQ(ln_history_state(session_, &can_undo, &can_redo), ln_result_ok);
    EXPECT_EQ(can_undo, 1);
    EXPECT_EQ(can_redo, 1);

    ASSERT_EQ(ln_undo(session_), ln_result_ok);
    EXPECT_TRUE(names().empty());
    ASSERT_EQ(ln_redo(session_), ln_result_ok);
    ASSERT_EQ(ln_redo(session_), ln_result_ok);
    EXPECT_EQ(new_names(), std::vector<std::string>({"a_x.txt"}));
    ASSERT_EQ(ln_history_state(session_, &can_undo, &can_redo), ln_result_ok);
    EXPECT_EQ(can_redo, 0);

    EXPECT_EQ(ln_undo(nullptr), ln_result_invalid_argument);
    EXPECT_EQ(ln_redo(nullptr), ln_result_invalid_argument);
    EXPECT_EQ(ln_history_state(session_, nullptr, &can_redo), ln_result_invalid_argument);
    EXPECT_EQ(ln_history_state(session_, &can_undo, nullptr), ln_result_invalid_argument);
}
