#include "rename_record_test.h"

TEST_F(RENAME_RECORD_TEST, keeps_entries)
{
    EXPECT_TRUE(record_.empty());
    record_.set({renamed(1, "a.txt", "b.txt")});
    EXPECT_EQ(record_.size(), size_t{1});
    record_.clear();
    EXPECT_TRUE(record_.empty());
}

TEST_F(RENAME_RECORD_TEST, prepares_every_restorable_entry)
{
    const UNDO_CHECK result = check({renamed(1, "a.txt", "x.txt"), renamed(2, "b.txt", "y.txt")});
    EXPECT_TRUE(result.skipped.empty());
    ASSERT_EQ(result.requests.size(), size_t{2});
    EXPECT_EQ(result.requests[0].item_id, 1u);
    EXPECT_EQ(result.requests[0].path, env_.root() / "x.txt");
    EXPECT_EQ(result.requests[0].name, U"x.txt");
    EXPECT_EQ(result.requests[0].new_name, U"a.txt");
}

TEST_F(RENAME_RECORD_TEST, skips_entries_moved_or_deleted_elsewhere)
{
    const RECORD_ENTRY moved = renamed(1, "a.txt", "x.txt");
    env_.fake().remove(moved.after);
    const UNDO_CHECK result = check({moved, renamed(2, "b.txt", "y.txt")});
    EXPECT_EQ(reasons(result), std::vector<RENAME_FAILURE>({RENAME_FAILURE::not_found}));
    EXPECT_EQ(result.skipped[0].item_id, 1u);
    EXPECT_EQ(result.skipped[0].stage, REPORT_STAGE::check);
    EXPECT_EQ(result.skipped[0].current, env_.root() / "x.txt");
    EXPECT_EQ(result.skipped[0].to, env_.root() / "a.txt");
    EXPECT_EQ(result.requests.size(), size_t{1});
}

TEST_F(RENAME_RECORD_TEST, skips_entries_replaced_by_another_file)
{
    const RECORD_ENTRY replaced = renamed(1, "a.txt", "x.txt");
    env_.fake().remove(replaced.after);
    env_.make_file(replaced.after, 3);
    EXPECT_EQ(reasons(check({replaced})),
        std::vector<RENAME_FAILURE>({RENAME_FAILURE::replaced}));
}

TEST_F(RENAME_RECORD_TEST, compares_size_and_time_without_file_ids)
{
    RECORD_ENTRY entry = renamed(1, "a.txt", "x.txt");
    entry.info.id.valid = false;
    EXPECT_TRUE(check({entry}).skipped.empty());
    entry.info.size = 99;
    EXPECT_EQ(reasons(check({entry})), std::vector<RENAME_FAILURE>({RENAME_FAILURE::replaced}));
    FILE_INFO folder;
    folder.is_dir = true;
    FILE_INFO other_folder;
    other_folder.is_dir = true;
    other_folder.modified_ms = 5;
    EXPECT_TRUE(RENAME_RECORD::same_file(folder, other_folder));
}

TEST_F(RENAME_RECORD_TEST, skips_entries_whose_old_name_is_taken)
{
    const RECORD_ENTRY entry = renamed(1, "a.txt", "x.txt");
    env_.make_file(env_.root() / "A.TXT", 1);
    EXPECT_EQ(reasons(check({entry})),
        std::vector<RENAME_FAILURE>({RENAME_FAILURE::already_exists}));
}

TEST_F(RENAME_RECORD_TEST, allows_swaps_back)
{
    const UNDO_CHECK result = check({renamed(1, "a.txt", "b.txt"),
        renamed(2, "b.txt", "a.txt")});
    EXPECT_TRUE(result.skipped.empty());
    EXPECT_EQ(result.requests.size(), size_t{2});
}

TEST_F(RENAME_RECORD_TEST, skips_entries_waiting_for_a_skipped_one)
{
    const RECORD_ENTRY first = renamed(1, "a.txt", "b.txt");
    const RECORD_ENTRY second = renamed(2, "b.txt", "a.txt");
    env_.fake().remove(first.after);
    env_.make_file(first.after, 3);
    const UNDO_CHECK result = check({first, second});
    EXPECT_EQ(reasons(result), std::vector<RENAME_FAILURE>(
        {RENAME_FAILURE::replaced, RENAME_FAILURE::depends_on_failed}));
    EXPECT_TRUE(result.requests.empty());
}

TEST_F(RENAME_RECORD_TEST, allows_case_only_changes_back)
{
    EXPECT_TRUE(check({renamed(1, "photo.jpg", "Photo.jpg")}).skipped.empty());
}

TEST_F(RENAME_RECORD_TEST, restores_inside_renamed_folders)
{
    const RECORD_ENTRY folder = renamed(1, "f", "g", true);
    const RECORD_ENTRY file = renamed(2, "f/x.txt", "g/y.txt");
    const UNDO_CHECK result = check({folder, file});
    EXPECT_TRUE(result.skipped.empty());
    ASSERT_EQ(result.requests.size(), size_t{2});
    EXPECT_TRUE(result.requests[0].is_dir);
    EXPECT_EQ(result.requests[1].path, env_.root() / "g" / "y.txt");
    EXPECT_EQ(result.requests[1].new_name, U"x.txt");
}
