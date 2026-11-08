#include "example_finder_test.h"

namespace
{
const char *const INSERT_X = R"({"kind":"insert","position":{"type":"start"},
    "value":[{"text":"x_"}]})";
const char *const REPLACE_TAG = R"({"kind":"replace","position":{"type":"text","text":"_tag"},
    "value":[{"text":""}]})";
}

TEST_F(EXAMPLE_FINDER_TEST, uses_the_default_name_for_an_empty_list)
{
    const EXAMPLE result = example(INSERT_X);
    EXPECT_EQ(result.item_id, 0u);
    EXPECT_EQ(result.index, 0u);
    EXPECT_EQ(result.before, U"IMG_0001.JPG");
    EXPECT_EQ(result.after, U"x_IMG_0001.JPG");
    EXPECT_EQ(result.state, EXAMPLE_STATE::changed);
}

TEST_F(EXAMPLE_FINDER_TEST, applies_earlier_modifications_to_the_default_name)
{
    mods_.add(MOD_JSON::parse(R"({"kind":"format","part":"full","position":{"type":"all"},
        "format":{"type":"lower"}})"));
    const EXAMPLE result = example(INSERT_X);
    EXPECT_EQ(result.before, U"img_0001.jpg");
    EXPECT_EQ(result.after, U"x_img_0001.jpg");
}

TEST_F(EXAMPLE_FINDER_TEST, picks_the_first_item_the_draft_changes)
{
    const std::vector<uint64_t> ids = add({U"a.txt", U"b_tag.txt", U"c_tag.txt"});
    const EXAMPLE result = example(REPLACE_TAG);
    EXPECT_EQ(result.item_id, ids[1]);
    EXPECT_EQ(result.index, 2u);
    EXPECT_EQ(result.before, U"b_tag.txt");
    EXPECT_EQ(result.after, U"b.txt");
    EXPECT_EQ(result.state, EXAMPLE_STATE::changed);
    EXPECT_EQ(result.prefix_len, 1u);
    EXPECT_EQ(result.suffix_len, 4u);
}

TEST_F(EXAMPLE_FINDER_TEST, falls_back_to_the_first_target)
{
    const std::vector<uint64_t> ids = add({U"a.txt", U"b.txt"});
    const EXAMPLE result = example(REPLACE_TAG);
    EXPECT_EQ(result.item_id, ids[0]);
    EXPECT_EQ(result.index, 1u);
    EXPECT_EQ(result.before, result.after);
    EXPECT_EQ(result.state, EXAMPLE_STATE::unchanged);
}

TEST_F(EXAMPLE_FINDER_TEST, looks_only_at_selected_items)
{
    const std::vector<uint64_t> ids = add({U"a_tag.txt", U"b.txt", U"c_tag.txt"});
    const EXAMPLE result = example(R"({"kind":"replace","position":{"type":"text",
        "text":"_tag"},"value":[{"text":""}],"scope":"items","targets":[)"
        + std::to_string(ids[2]) + "]}");
    EXPECT_EQ(result.item_id, ids[2]);
    EXPECT_EQ(result.index, 3u);
    EXPECT_EQ(result.state, EXAMPLE_STATE::changed);
}

TEST_F(EXAMPLE_FINDER_TEST, reports_missing_targets)
{
    add({U"a.txt"});
    const EXAMPLE result = example(R"({"kind":"insert","position":{"type":"start"},
        "value":[{"text":"x"}],"scope":"items","targets":[999]})");
    EXPECT_EQ(result.state, EXAMPLE_STATE::no_target);
    EXPECT_EQ(result.item_id, 0u);
    EXPECT_TRUE(result.before.empty());
}

TEST_F(EXAMPLE_FINDER_TEST, skips_folders_for_extension_changes)
{
    add({U"folder"}, true);
    const std::vector<uint64_t> files = add({U"photo.jpg"});
    const char *upper = R"({"kind":"format","part":"ext","position":{"type":"all"},
        "format":{"type":"upper"}})";
    const EXAMPLE result = example(upper);
    EXPECT_EQ(result.item_id, files[0]);
    EXPECT_EQ(result.after, U"photo.JPG");
    items_.remove(files);
    EXPECT_EQ(example(upper).state, EXAMPLE_STATE::skipped_dir);
}

TEST_F(EXAMPLE_FINDER_TEST, starts_from_modifications_before_the_edited_one)
{
    add({U"name.txt"});
    const uint64_t first = mods_.add(MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"A_"}]})"));
    const uint64_t second = mods_.add(MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"B_"}]})"));
    const uint64_t off = mods_.add(MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"OFF_"}]})"));
    mods_.set_enabled(off, false);
    const char *edited = R"({"kind":"insert","position":{"type":"start"},
        "value":[{"text":"C_"}]})";
    EXPECT_EQ(example(edited, second).before, U"A_name.txt");
    EXPECT_EQ(example(edited, second).after, U"C_A_name.txt");
    EXPECT_EQ(example(edited, first).before, U"name.txt");
    EXPECT_EQ(example(edited).before, U"B_A_name.txt");
    EXPECT_FALSE(finder_.find(items_, mods_.entries(), MOD_JSON::parse(edited), 999).has_value());
}

TEST_F(EXAMPLE_FINDER_TEST, numbers_within_the_draft_scope)
{
    const std::vector<uint64_t> ids = add({U"a.txt", U"b.txt", U"c.txt"});
    const EXAMPLE result = example(R"({"kind":"insert","position":{"type":"start"},
        "value":[{"token":"number"},{"text":"_"}],"scope":"items","targets":[)"
        + std::to_string(ids[1]) + "," + std::to_string(ids[2]) + "]}");
    EXPECT_EQ(result.after, U"1_b.txt");
}

TEST_F(EXAMPLE_FINDER_TEST, reports_name_errors)
{
    add({U"a.txt"});
    const EXAMPLE result = example(R"({"kind":"insert","position":{"type":"start"},
        "value":[{"text":"x:"}]})");
    EXPECT_TRUE(result.errors & NAME_ISSUE::WIN_FORBIDDEN_CHAR);
}
