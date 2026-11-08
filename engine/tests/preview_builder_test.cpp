#include "preview_builder_test.h"

TEST_F(PREVIEW_BUILDER_TEST, keeps_names_without_modifications)
{
    add_files("photos", {U"a.jpg", U"b.jpg"});
    const PREVIEW preview = build();
    ASSERT_EQ(preview.rows.size(), size_t{2});
    for (const PREVIEW_ROW &row : preview.rows) {
        EXPECT_FALSE(row.changed);
        EXPECT_EQ(row.status, ROW_STATUS::unchanged);
        EXPECT_EQ(row.new_name, row.name);
        EXPECT_EQ(row.prefix_len, row.name.size());
    }
    EXPECT_EQ(preview.changed, 0u);
    EXPECT_EQ(preview.runnable, 0u);
}

TEST_F(PREVIEW_BUILDER_TEST, fills_item_details)
{
    const std::vector<uint64_t> ids = add_files("photos", {U"a.jpg"});
    const PREVIEW_ROW row = build().rows[0];
    EXPECT_EQ(row.item_id, ids[0]);
    EXPECT_EQ(row.folder, env_.root() / "photos");
    EXPECT_EQ(row.name, U"a.jpg");
    EXPECT_EQ(row.size, 10);
    EXPECT_FALSE(row.is_dir);
}

TEST_F(PREVIEW_BUILDER_TEST, applies_enabled_modifications_in_order)
{
    add_files("photos", {U"IMG_7.JPG"});
    add_mod(R"({"kind":"replace","position":{"type":"text","text":"IMG_"},
        "value":[{"text":"trip_"}]})");
    const uint64_t off = add_mod(
        R"({"kind":"insert","position":{"type":"end"},"value":[{"text":"_off"}]})");
    add_mod(R"({"kind":"format","part":"ext","position":{"type":"all"},
        "format":{"type":"lower"}})");
    mods_.set_enabled(off, false);
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"trip_7.jpg"}));
}

TEST_F(PREVIEW_BUILDER_TEST, numbers_across_folders_in_list_order)
{
    add_files("photos", {U"a.jpg", U"b.jpg"});
    add_files("docs", {U"c.txt"});
    add_mod(R"({"kind":"insert","position":{"type":"start"},
        "value":[{"token":"number"},{"text":"_"}]})");
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"1_a.jpg", U"2_b.jpg", U"3_c.txt"}));
}

TEST_F(PREVIEW_BUILDER_TEST, applies_to_selected_items_only)
{
    const std::vector<uint64_t> ids = add_files("photos", {U"a.jpg", U"b.jpg", U"c.jpg"});
    add_mod(R"({"kind":"insert","position":{"type":"start"},
        "value":[{"token":"number"},{"text":"_"}],"scope":"items","targets":[)"
        + std::to_string(ids[2]) + "," + std::to_string(ids[0]) + "]}");
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"1_a.jpg", U"b.jpg", U"2_c.jpg"}));
}

TEST_F(PREVIEW_BUILDER_TEST, ignores_targets_outside_the_list)
{
    const std::vector<uint64_t> ids = add_files("photos", {U"a.jpg", U"b.jpg"});
    add_mod(R"({"kind":"insert","position":{"type":"start"},
        "value":[{"token":"number"},{"text":"_"}],"scope":"items","targets":[)"
        + std::to_string(ids[0]) + "," + std::to_string(ids[1]) + ",999]}");
    items_.remove({ids[0]});
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"1_b.jpg"}));
}

TEST_F(PREVIEW_BUILDER_TEST, fills_item_tokens)
{
    add_files("photos", {U"trip.final.jpg"});
    add_mod(R"({"kind":"replace","position":{"type":"all"},"value":[{"token":"parent_folder"},
        {"text":"_"},{"token":"original_name"},{"text":"_"},{"token":"size"}],
        "tokens":{"size":{"unit":"b"}}})");
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"photos_trip.final_10B.jpg"}));
}

TEST_F(PREVIEW_BUILDER_TEST, treats_normalization_differences_as_unchanged)
{
    const std::filesystem::path nfd = env_.root() / "photos" / u8"café.txt";
    env_.make_file(nfd, 1);
    items_.add({nfd}, ADD_MODE::as_items);
    add_mod(R"({"kind":"replace","position":{"type":"text","text":"café"},
        "value":[{"text":"café"}]})");
    const PREVIEW_ROW row = build().rows[0];
    EXPECT_FALSE(row.changed);
    EXPECT_EQ(row.status, ROW_STATUS::unchanged);
}

TEST_F(PREVIEW_BUILDER_TEST, treats_case_changes_as_changes)
{
    add_files("photos", {U"photo.jpg"});
    add_mod(R"({"kind":"format","position":{"type":"all"},"format":{"type":"upper"}})");
    const PREVIEW preview = build();
    EXPECT_TRUE(preview.rows[0].changed);
    EXPECT_EQ(preview.rows[0].status, ROW_STATUS::ok);
    EXPECT_EQ(preview.runnable, 1u);
}

TEST_F(PREVIEW_BUILDER_TEST, reports_errors_and_warnings)
{
    add_files("photos", {U"a.jpg", U"b.jpg", U"c.jpg"});
    const std::vector<uint64_t> order = items_.order();
    mods_.direct_edit(order[0], U"a:b.jpg");
    mods_.direct_edit(order[1], U".hidden");
    const PREVIEW preview = build();
    EXPECT_EQ(preview.rows[0].status, ROW_STATUS::error);
    EXPECT_TRUE(preview.rows[0].errors & NAME_ISSUE::WIN_FORBIDDEN_CHAR);
    EXPECT_EQ(preview.rows[1].status, ROW_STATUS::warning);
    EXPECT_TRUE(preview.rows[1].warnings & NAME_ISSUE::HIDDEN);
    EXPECT_EQ(preview.rows[2].status, ROW_STATUS::unchanged);
    EXPECT_EQ(preview.changed, 2u);
    EXPECT_EQ(preview.error, 1u);
    EXPECT_EQ(preview.warning, 1u);
    EXPECT_EQ(preview.runnable, 1u);
}

TEST_F(PREVIEW_BUILDER_TEST, reports_conflicts_with_positions)
{
    add_files("photos", {U"a.jpg", U"b.jpg", U"keep.jpg"});
    env_.make_file(env_.root() / "photos" / "outside.jpg", 1);
    cache_.reload(env_.root() / "photos");
    const std::vector<uint64_t> order = items_.order();
    mods_.direct_edit(order[0], U"keep.jpg");
    mods_.direct_edit(order[1], U"outside.jpg");
    const PREVIEW preview = build();
    EXPECT_EQ(preview.rows[0].status, ROW_STATUS::conflict);
    EXPECT_EQ(preview.rows[0].conflict, CONFLICT_KIND::list_item);
    EXPECT_EQ(preview.rows[0].conflict_index, 3u);
    EXPECT_EQ(preview.rows[1].conflict, CONFLICT_KIND::existing_item);
    EXPECT_EQ(preview.rows[1].conflict_index, 0u);
    EXPECT_EQ(preview.conflict, 2u);
    EXPECT_EQ(preview.runnable, 0u);
}

TEST_F(PREVIEW_BUILDER_TEST, adjusts_duplicates_when_enabled)
{
    add_files("photos", {U"a.jpg", U"b.jpg"});
    add_mod(R"({"kind":"replace","position":{"type":"all"},"value":[{"text":"same"}]})");
    EXPECT_EQ(build().conflict, 2u);
    options_.duplicate.enabled = true;
    const PREVIEW preview = build();
    EXPECT_EQ(preview.rows[0].new_name, U"same.jpg");
    EXPECT_EQ(preview.rows[1].new_name, U"same (1).jpg");
    EXPECT_FALSE(preview.rows[0].adjusted);
    EXPECT_TRUE(preview.rows[1].adjusted);
    EXPECT_EQ(preview.conflict, 0u);
    EXPECT_EQ(preview.runnable, 2u);
}

TEST_F(PREVIEW_BUILDER_TEST, marks_enabled_direct_edits)
{
    add_files("photos", {U"a.jpg", U"b.jpg"});
    const std::vector<uint64_t> order = items_.order();
    const uint64_t direct = mods_.direct_edit(order[0], U"x.jpg");
    EXPECT_TRUE(build().rows[0].direct);
    EXPECT_FALSE(build().rows[1].direct);
    mods_.set_enabled(direct, false);
    EXPECT_FALSE(build().rows[0].direct);
}

TEST_F(PREVIEW_BUILDER_TEST, measures_the_changed_part)
{
    add_files("photos", {U"IMG_0001.jpg"});
    add_mod(R"({"kind":"replace","position":{"type":"text","text":"IMG"},
        "value":[{"text":"trip"}]})");
    const PREVIEW_ROW row = build().rows[0];
    EXPECT_EQ(row.new_name, U"trip_0001.jpg");
    EXPECT_EQ(row.prefix_len, 0u);
    EXPECT_EQ(row.suffix_len, 9u);
}

TEST_F(PREVIEW_BUILDER_TEST, measures_insertions)
{
    add_files("photos", {U"aa.jpg"});
    add_mod(R"({"kind":"insert","position":{"type":"after_nth","n":1},"value":[{"text":"a"}]})");
    const PREVIEW_ROW row = build().rows[0];
    EXPECT_EQ(row.new_name, U"aaa.jpg");
    EXPECT_EQ(row.prefix_len + row.suffix_len, row.name.size());
}
