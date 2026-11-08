#include "history_test.h"

TEST_F(HISTORY_TEST, starts_empty)
{
    EXPECT_FALSE(history_.can_undo());
    EXPECT_FALSE(history_.can_redo());
    EXPECT_FALSE(history_.undo(state(1)).has_value());
    EXPECT_FALSE(history_.redo(state(1)).has_value());
}

TEST_F(HISTORY_TEST, skips_steps_that_change_nothing)
{
    history_.record(state(1), state(1));
    EXPECT_FALSE(history_.can_undo());
}

TEST_F(HISTORY_TEST, undoes_and_redoes_steps)
{
    history_.record(state(1), state(2));
    history_.record(state(2), state(3));
    EXPECT_EQ(history_.undo(state(3)), state(2));
    EXPECT_EQ(history_.undo(state(2)), state(1));
    EXPECT_FALSE(history_.can_undo());
    EXPECT_EQ(history_.redo(state(1)), state(2));
    EXPECT_EQ(history_.redo(state(2)), state(3));
    EXPECT_FALSE(history_.can_redo());
    EXPECT_TRUE(history_.can_undo());
}

TEST_F(HISTORY_TEST, drops_redo_after_a_new_step)
{
    history_.record(state(1), state(2));
    history_.undo(state(2));
    EXPECT_TRUE(history_.can_redo());
    history_.record(state(1), state(5));
    EXPECT_FALSE(history_.can_redo());
    EXPECT_EQ(history_.undo(state(5)), state(1));
}

TEST_F(HISTORY_TEST, keeps_the_latest_steps_only)
{
    for (uint64_t step = 0; step <= HISTORY::MAX_STEPS; ++step) {
        history_.record(state(step), state(step + 1));
    }
    uint64_t current = HISTORY::MAX_STEPS + 1;
    size_t undone = 0;
    while (history_.can_undo()) {
        const std::optional<SNAPSHOT> previous = history_.undo(state(current));
        ASSERT_TRUE(previous.has_value());
        current = previous->order[0];
        ++undone;
    }
    EXPECT_EQ(undone, HISTORY::MAX_STEPS);
    EXPECT_EQ(current, 1u);
}

TEST_F(HISTORY_TEST, clears_both_directions)
{
    history_.record(state(1), state(2));
    history_.record(state(2), state(3));
    history_.undo(state(3));
    history_.clear();
    EXPECT_FALSE(history_.can_undo());
    EXPECT_FALSE(history_.can_redo());
}
