#include "session_rename_test.h"

namespace
{
const char *const PREFIX =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a_"}]})";

std::vector<std::u32string> item_names(const SESSION &session)
{
    std::vector<std::u32string> names;
    for (const uint64_t id : session.items().order()) {
        names.push_back(session.items().find(id)->name);
    }
    return names;
}
}

TEST_F(SESSION_TEST, renames_and_finishes)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    const std::optional<RENAME_REPORT> report = session_->rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->succeeded, 2u);
    EXPECT_FALSE(report->needs_choice);
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"a_x.txt", U"a_y.txt"}));
    EXPECT_EQ(session_->items().find(session_->items().order()[0])->path, root_ / "a_x.txt");
    EXPECT_FALSE(session_->mods()[0].enabled);
    EXPECT_EQ(session_->preview().changed, 0u);
    EXPECT_FALSE(session_->can_undo());
    ASSERT_EQ(session_->record().size(), size_t{2});
    EXPECT_EQ(session_->record().entries()[0].before, root_ / "x.txt");
    EXPECT_EQ(session_->record().entries()[0].after, root_ / "a_x.txt");
    EXPECT_TRUE(session_->record().entries()[0].info.id.valid);
    FILE_INFO info;
    EXPECT_TRUE(fake_->get_info(root_ / "a_y.txt", info).ok());
}

TEST_F(SESSION_TEST, refuses_to_run_with_errors_or_conflicts)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    session_->direct_edit(session_->items().order()[0], U"y.txt");
    EXPECT_FALSE(session_->rename({}).has_value());
    session_->undo();
    session_->direct_edit(session_->items().order()[0], U"a:b.txt");
    EXPECT_FALSE(session_->rename({}).has_value());
    FILE_INFO info;
    EXPECT_TRUE(fake_->get_info(root_ / "x.txt", info).ok());
    EXPECT_TRUE(session_->mods()[0].enabled);
}

TEST_F(SESSION_TEST, checks_folders_again_before_running)
{
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    session_->direct_edit(session_->items().order()[0], U"late.txt");
    EXPECT_EQ(session_->preview().conflict, 0u);
    files({U"late.txt"});
    EXPECT_FALSE(session_->rename({}).has_value());
}

TEST_F(SESSION_TEST, does_nothing_without_changes)
{
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    const std::optional<RENAME_REPORT> report = session_->rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->total, 0u);
    EXPECT_TRUE(session_->record().empty());
    EXPECT_TRUE(session_->can_undo());
}

TEST_F(SESSION_TEST, allows_warnings)
{
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    session_->direct_edit(session_->items().order()[0], U".hidden");
    EXPECT_EQ(session_->preview().warning, 1u);
    EXPECT_EQ(session_->rename({})->succeeded, 1u);
}

TEST_F(SESSION_TEST, waits_for_a_choice_after_failures)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    fake_->fail_next_rename(root_ / "y.txt", FS_ERROR::in_use);
    const std::optional<RENAME_REPORT> report = session_->rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_TRUE(report->needs_choice);
    EXPECT_TRUE(session_->rename_pending());
    EXPECT_THROW(session_->add_mod(mod(PREFIX)), SESSION_BUSY);
    EXPECT_THROW(session_->undo(), SESSION_BUSY);
    EXPECT_THROW(session_->rename({}), SESSION_BUSY);
    EXPECT_EQ(session_->preview().rows.size(), size_t{2});

    const RENAME_REPORT kept = session_->resolve_rename(RESOLVE_CHOICE::keep_succeeded, {});
    EXPECT_EQ(kept.succeeded, 1u);
    EXPECT_FALSE(session_->rename_pending());
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"a_x.txt", U"y.txt"}));
    ASSERT_EQ(session_->record().size(), size_t{1});
    EXPECT_FALSE(session_->mods()[0].enabled);
    EXPECT_THROW(session_->resolve_rename(RESOLVE_CHOICE::keep_succeeded, {}), std::logic_error);
}

TEST_F(SESSION_TEST, treats_a_full_revert_as_not_run)
{
    session_->add_items(files({U"first.txt"}), ADD_MODE::as_items);
    session_->direct_edit(session_->items().order()[0], U"done.txt");
    session_->rename({});
    ASSERT_EQ(session_->record().size(), size_t{1});

    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    const uint64_t prefix = session_->add_mod(mod(PREFIX));
    fake_->fail_next_rename(root_ / "y.txt", FS_ERROR::in_use);
    session_->rename({});
    session_->resolve_rename(RESOLVE_CHOICE::revert_all, {});
    EXPECT_EQ(item_names(*session_),
        std::vector<std::u32string>({U"done.txt", U"x.txt", U"y.txt"}));
    FILE_INFO info;
    EXPECT_TRUE(fake_->get_info(root_ / "x.txt", info).ok());
    EXPECT_TRUE(fake_->get_info(root_ / "done.txt", info).ok());
    for (const MOD_ENTRY &entry : session_->mods()) {
        EXPECT_EQ(entry.enabled, entry.id == prefix);
    }
    EXPECT_TRUE(session_->can_undo());
    ASSERT_EQ(session_->record().size(), size_t{1});
    EXPECT_EQ(session_->record().entries()[0].after, root_ / "done.txt");
}

TEST_F(SESSION_TEST, updates_items_inside_renamed_folders)
{
    fake_->add_folder(root_ / "f");
    fake_->add_file(root_ / "f" / "keep.txt", 1);
    fake_->add_file(root_ / "f" / "b.txt", 1);
    session_->add_items({root_ / "f", root_ / "f" / "keep.txt", root_ / "f" / "b.txt"},
        ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    session_->direct_edit(ids[0], U"g");
    session_->direct_edit(ids[2], U"b2.txt");
    EXPECT_EQ(session_->rename({})->succeeded, 2u);
    EXPECT_EQ(session_->items().find(ids[0])->path, root_ / "g");
    EXPECT_EQ(session_->items().find(ids[1])->path, root_ / "g" / "keep.txt");
    EXPECT_EQ(session_->items().find(ids[1])->folder, root_ / "g");
    EXPECT_EQ(session_->items().find(ids[2])->path, root_ / "g" / "b2.txt");
    EXPECT_EQ(session_->record().entries()[1].after, root_ / "g" / "b2.txt");
    EXPECT_EQ(session_->add_items({root_ / "g" / "keep.txt"}, ADD_MODE::as_items).skipped, 1u);
}

TEST_F(SESSION_TEST, checks_the_record_before_undoing)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    session_->rename({});
    fake_->remove(root_ / "a_x.txt");
    const RENAME_REPORT report = session_->undo_check();
    EXPECT_EQ(report.total, 2u);
    EXPECT_EQ(report.skipped, 1u);
    ASSERT_EQ(report.problems.size(), size_t{1});
    EXPECT_EQ(report.problems[0].failure, RENAME_FAILURE::not_found);
    FILE_INFO info;
    EXPECT_TRUE(fake_->get_info(root_ / "a_y.txt", info).ok());
}

TEST_F(SESSION_TEST, undoes_the_last_rename)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    session_->rename({});
    std::vector<uint32_t> progress;
    const std::optional<RENAME_REPORT> report =
        session_->undo_rename([&](uint32_t done, uint32_t) { progress.push_back(done); });
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->total, 2u);
    EXPECT_EQ(report->succeeded, 2u);
    EXPECT_TRUE(report->problems.empty());
    EXPECT_FALSE(report->needs_choice);
    EXPECT_EQ(progress, std::vector<uint32_t>({1, 2}));
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"x.txt", U"y.txt"}));
    FILE_INFO info;
    EXPECT_TRUE(fake_->get_info(root_ / "x.txt", info).ok());
    EXPECT_TRUE(session_->record().empty());
    EXPECT_FALSE(session_->mods()[0].enabled);
    EXPECT_FALSE(session_->can_undo());
    EXPECT_FALSE(session_->undo_rename({}).has_value());
}

TEST_F(SESSION_TEST, undoes_by_the_record_after_list_changes)
{
    session_->add_items(files({U"x.txt", U"y.txt", U"z.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    session_->rename({});
    const std::vector<uint64_t> ids = session_->items().order();
    session_->remove_items({ids[0]});
    session_->move_items({ids[2]}, ids[1], false, false);
    const uint64_t later = session_->add_mod(mod(PREFIX));
    EXPECT_EQ(session_->undo_rename({})->succeeded, 3u);
    FILE_INFO info;
    EXPECT_TRUE(fake_->get_info(root_ / "x.txt", info).ok());
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"z.txt", U"y.txt"}));
    EXPECT_TRUE(session_->mods().back().enabled);
    EXPECT_EQ(session_->mods().back().id, later);
}

TEST_F(SESSION_TEST, skips_and_keeps_entries_that_cannot_be_undone)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    session_->rename({});
    fake_->remove(root_ / "a_x.txt");
    const std::optional<RENAME_REPORT> report = session_->undo_rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->succeeded, 1u);
    EXPECT_EQ(report->skipped, 1u);
    ASSERT_EQ(report->problems.size(), size_t{1});
    EXPECT_EQ(report->problems[0].stage, REPORT_STAGE::check);
    ASSERT_EQ(session_->record().size(), size_t{1});
    EXPECT_FALSE(session_->undo_rename({}).has_value());
}

TEST_F(SESSION_TEST, retries_entries_that_failed_to_undo)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    session_->rename({});
    fake_->fail_next_rename(root_ / "a_y.txt", FS_ERROR::in_use);
    const std::optional<RENAME_REPORT> report = session_->undo_rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->succeeded, 1u);
    EXPECT_EQ(report->failed, 1u);
    EXPECT_FALSE(report->needs_choice);
    EXPECT_FALSE(session_->rename_pending());
    ASSERT_EQ(session_->record().size(), size_t{1});
    EXPECT_EQ(session_->undo_rename({})->succeeded, 1u);
    EXPECT_TRUE(session_->record().empty());
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"x.txt", U"y.txt"}));
}

TEST_F(SESSION_TEST, undoes_swaps)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    session_->direct_edit(ids[0], U"y.txt");
    session_->direct_edit(ids[1], U"x.txt");
    session_->rename({});
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"y.txt", U"x.txt"}));
    EXPECT_TRUE(session_->items().contains(ids[0]));
    EXPECT_TRUE(session_->items().contains(ids[1]));
    EXPECT_EQ(session_->undo_rename({})->succeeded, 2u);
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"x.txt", U"y.txt"}));
    EXPECT_TRUE(session_->items().contains(ids[0]));
    EXPECT_TRUE(session_->items().contains(ids[1]));
}

TEST_F(SESSION_TEST, moves_items_of_swapped_folders_once)
{
    fake_->add_folder(root_ / "a");
    fake_->add_folder(root_ / "b");
    fake_->add_file(root_ / "a" / "x.txt", 1);
    fake_->add_file(root_ / "b" / "x.txt", 2);
    session_->add_items({root_ / "a", root_ / "b", root_ / "a" / "x.txt", root_ / "b" / "x.txt"},
        ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    session_->direct_edit(ids[0], U"b");
    session_->direct_edit(ids[1], U"a");
    EXPECT_EQ(session_->rename({})->succeeded, 2u);
    EXPECT_EQ(session_->items().find(ids[2])->path, root_ / "b" / "x.txt");
    EXPECT_EQ(session_->items().find(ids[3])->path, root_ / "a" / "x.txt");
    EXPECT_EQ(session_->items().find(ids[2])->info.size, 1u);
    for (const uint64_t id : ids) {
        EXPECT_TRUE(session_->items().contains(id));
    }
    EXPECT_EQ(session_->undo_rename({})->succeeded, 2u);
    EXPECT_EQ(session_->items().find(ids[2])->path, root_ / "a" / "x.txt");
    EXPECT_EQ(session_->items().find(ids[3])->path, root_ / "b" / "x.txt");
    for (const uint64_t id : ids) {
        EXPECT_TRUE(session_->items().contains(id));
    }
}

TEST_F(SESSION_TEST, undoes_folders_and_their_items)
{
    fake_->add_folder(root_ / "f");
    fake_->add_file(root_ / "f" / "keep.txt", 1);
    fake_->add_file(root_ / "f" / "b.txt", 1);
    session_->add_items({root_ / "f", root_ / "f" / "keep.txt", root_ / "f" / "b.txt"},
        ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    session_->direct_edit(ids[0], U"g");
    session_->direct_edit(ids[2], U"b2.txt");
    session_->rename({});
    EXPECT_EQ(session_->undo_rename({})->succeeded, 2u);
    EXPECT_EQ(session_->items().find(ids[0])->path, root_ / "f");
    EXPECT_EQ(session_->items().find(ids[1])->path, root_ / "f" / "keep.txt");
    EXPECT_EQ(session_->items().find(ids[2])->path, root_ / "f" / "b.txt");
    FILE_INFO info;
    EXPECT_TRUE(fake_->get_info(root_ / "f" / "b.txt", info).ok());
}

TEST_F(SESSION_TEST, follows_restored_folders_for_entries_left_behind)
{
    fake_->add_folder(root_ / "f");
    fake_->add_file(root_ / "f" / "b.txt", 1);
    session_->add_items({root_ / "f", root_ / "f" / "b.txt"}, ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    session_->direct_edit(ids[0], U"g");
    session_->direct_edit(ids[1], U"b2.txt");
    session_->rename({});
    fake_->fail_next_rename(root_ / "g" / "b2.txt", FS_ERROR::in_use);
    EXPECT_EQ(session_->undo_rename({})->succeeded, 1u);
    ASSERT_EQ(session_->record().size(), size_t{1});
    EXPECT_EQ(session_->record().entries()[0].after, root_ / "f" / "b2.txt");
    EXPECT_EQ(session_->items().find(ids[1])->path, root_ / "f" / "b2.txt");
    EXPECT_EQ(session_->undo_rename({})->succeeded, 1u);
    EXPECT_EQ(session_->items().find(ids[1])->path, root_ / "f" / "b.txt");
}

TEST_F(SESSION_TEST, undoes_to_the_original_normalization_form)
{
    const std::u32string decomposed = U"\u1100\u1161.txt";
    session_->add_items(files({decomposed}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    session_->rename({});
    EXPECT_EQ(item_names(*session_), std::vector<std::u32string>({U"a_\uAC00.txt"}));
    EXPECT_EQ(session_->undo_rename({})->succeeded, 1u);
    std::vector<std::filesystem::path> names;
    ASSERT_TRUE(fake_->list_names(root_, names).ok());
    EXPECT_EQ(names, std::vector<std::filesystem::path>({UTF::path_from_u32(decomposed)}));
}

TEST_F(SESSION_TEST, reverts_to_the_original_normalization_form)
{
    const std::u32string decomposed = U"\u1100\u1161.txt";
    session_->add_items(files({decomposed, U"y.txt"}), ADD_MODE::as_items);
    session_->add_mod(mod(PREFIX));
    fake_->fail_next_rename(root_ / "y.txt", FS_ERROR::in_use);
    ASSERT_TRUE(session_->rename({})->needs_choice);
    session_->resolve_rename(RESOLVE_CHOICE::revert_all, {});
    std::vector<std::filesystem::path> names;
    ASSERT_TRUE(fake_->list_names(root_, names).ok());
    EXPECT_EQ(names, std::vector<std::filesystem::path>({"y.txt",
        UTF::path_from_u32(decomposed)}));
}
