#include "mod_checker_test.h"

namespace
{
std::vector<MOD_PROBLEM> problems_of(const char *json)
{
    return MOD_CHECKER::check(MOD_JSON::parse(json));
}
}

TEST(MOD_CHECKER, accepts_complete_modifications)
{
    EXPECT_TRUE(problems_of(
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a"}]})").empty());
    EXPECT_TRUE(problems_of(
        R"({"kind":"replace","position":{"type":"all"},"value":[{"token":"number"}]})").empty());
    EXPECT_TRUE(problems_of(R"({"kind":"delete","position":{"type":"all"}})").empty());
    EXPECT_TRUE(problems_of(
        R"({"kind":"format","position":{"type":"all"},"format":{"type":"upper"}})").empty());
    EXPECT_TRUE(problems_of(R"({"kind":"delete","position":{"type":"between",
        "from_beginning":true,"to_end":true}})").empty());
    EXPECT_TRUE(problems_of(R"({"kind":"delete","position":{"type":"all"},
        "scope":"items","targets":[1]})").empty());
}

TEST(MOD_CHECKER, requires_targets_for_items_scope)
{
    const std::vector<MOD_PROBLEM> expected = {{MOD_FIELD::targets, PROBLEM_REASON::required}};
    EXPECT_EQ(problems_of(R"({"kind":"delete","position":{"type":"all"},"scope":"items"})"),
        expected);
    EXPECT_EQ(problems_of(R"({"kind":"delete","position":{"type":"all"},"scope":"items",
        "targets":[]})"), expected);
}

TEST(MOD_CHECKER, requires_find_text)
{
    const std::vector<MOD_PROBLEM> expected = {{MOD_FIELD::find_text, PROBLEM_REASON::required}};
    EXPECT_EQ(problems_of(R"({"kind":"delete","position":{"type":"text","text":""}})"),
        expected);
    EXPECT_EQ(problems_of(R"({"kind":"insert","position":{"type":"after_text","text":""},
        "value":[{"text":"a"}]})"), expected);
}

TEST(MOD_CHECKER, requires_delimiters)
{
    const std::vector<MOD_PROBLEM> both = {
        {MOD_FIELD::start_text, PROBLEM_REASON::required},
        {MOD_FIELD::end_text, PROBLEM_REASON::required}};
    EXPECT_EQ(problems_of(R"({"kind":"delete","position":{"type":"between",
        "start_text":"","end_text":""}})"), both);
    const std::vector<MOD_PROBLEM> end = {{MOD_FIELD::end_text, PROBLEM_REASON::required}};
    EXPECT_EQ(problems_of(R"({"kind":"delete","position":{"type":"between",
        "from_beginning":true,"end_text":""}})"), end);
}

TEST(MOD_CHECKER, requires_value_for_insert_and_replace)
{
    const std::vector<MOD_PROBLEM> expected = {{MOD_FIELD::value, PROBLEM_REASON::required}};
    EXPECT_EQ(problems_of(R"({"kind":"insert","position":{"type":"start"},"value":[]})"),
        expected);
    EXPECT_EQ(problems_of(R"({"kind":"replace","position":{"type":"all"},
        "value":[{"text":""},{"text":""}]})"), expected);
}

TEST(MOD_CHECKER, reports_problems_in_editor_order)
{
    const std::vector<MOD_PROBLEM> expected = {
        {MOD_FIELD::targets, PROBLEM_REASON::required},
        {MOD_FIELD::find_text, PROBLEM_REASON::required},
        {MOD_FIELD::value, PROBLEM_REASON::required}};
    EXPECT_EQ(problems_of(R"({"kind":"replace","position":{"type":"text","text":""},
        "value":[],"scope":"items"})"), expected);
}
