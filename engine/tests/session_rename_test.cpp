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
