#include "session_real_fs_test.h"

namespace
{
const char *const PREFIX =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a_"}]})";

const char *const NUMBER =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"token":"number"},{"text":"_"}],
    "tokens":{"number":{"digits":2}}})";
}

TEST_F(SESSION_REAL_FS_TEST, renames_and_undoes_folder_contents)
{
    files({U"b.txt", U"a.txt", U"c.txt"});
    EXPECT_EQ(session_.add_items({env_.root()}, ADD_MODE::folder_contents).added, 3u);
    session_.add_mod(mod(NUMBER));
    const std::optional<RENAME_REPORT> report = session_.rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->succeeded, 3u);
    EXPECT_EQ(names_in(env_.root()), sorted({"01_a.txt", "02_b.txt", "03_c.txt"}));
    EXPECT_EQ(item_names(), std::vector<std::u32string>({U"01_a.txt", U"02_b.txt", U"03_c.txt"}));
    EXPECT_EQ(session_.record().size(), size_t{3});

    EXPECT_EQ(session_.undo_rename({})->succeeded, 3u);
    EXPECT_EQ(names_in(env_.root()), sorted({"a.txt", "b.txt", "c.txt"}));
    EXPECT_EQ(item_names(), std::vector<std::u32string>({U"a.txt", U"b.txt", U"c.txt"}));
    EXPECT_TRUE(session_.record().empty());
}

TEST_F(SESSION_REAL_FS_TEST, renames_cycles_and_case_changes)
{
    session_.add_items(files({U"a.txt", U"b.txt", U"c.txt", U"d.txt"}), ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_.items().order();
    session_.direct_edit(ids[0], U"b.txt");
    session_.direct_edit(ids[1], U"c.txt");
    session_.direct_edit(ids[2], U"a.txt");
    session_.direct_edit(ids[3], U"D.txt");
    EXPECT_EQ(session_.rename({})->succeeded, 4u);
    EXPECT_EQ(names_in(env_.root()), sorted({"D.txt", "a.txt", "b.txt", "c.txt"}));
    EXPECT_EQ(size_of(at("b.txt")), 1);
    EXPECT_EQ(size_of(at("c.txt")), 2);
    EXPECT_EQ(size_of(at("a.txt")), 3);

    EXPECT_EQ(session_.undo_rename({})->succeeded, 4u);
    EXPECT_EQ(names_in(env_.root()), sorted({"a.txt", "b.txt", "c.txt", "d.txt"}));
    EXPECT_EQ(size_of(at("a.txt")), 1);
    EXPECT_EQ(size_of(at("c.txt")), 3);
}

TEST_F(SESSION_REAL_FS_TEST, renames_folders_with_their_items)
{
    env_.make_folder(at("f"));
    env_.make_file(at("f/keep.txt"), 1);
    env_.make_file(at("f/x.txt"), 1);
    session_.add_items({at("f"), at("f/keep.txt"), at("f/x.txt")}, ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_.items().order();
    session_.direct_edit(ids[0], U"g");
    session_.direct_edit(ids[2], U"y.txt");
    EXPECT_EQ(session_.rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({"g"}));
    EXPECT_EQ(names_in(at("g")), sorted({"keep.txt", "y.txt"}));
    EXPECT_EQ(session_.items().find(ids[1])->path, at("g/keep.txt"));
    EXPECT_EQ(session_.items().find(ids[2])->path, at("g/y.txt"));

    EXPECT_EQ(session_.undo_rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({"f"}));
    EXPECT_EQ(names_in(at("f")), sorted({"keep.txt", "x.txt"}));
    EXPECT_EQ(session_.items().find(ids[1])->path, at("f/keep.txt"));
    EXPECT_EQ(session_.items().find(ids[2])->path, at("f/x.txt"));
}

TEST_F(SESSION_REAL_FS_TEST, undoes_to_the_original_normalization_form)
{
    const std::u32string decomposed = U"가.txt";
    session_.add_items(files({decomposed}), ADD_MODE::as_items);
    session_.add_mod(mod(PREFIX));
    EXPECT_EQ(session_.rename({})->succeeded, 1u);
    EXPECT_EQ(names_in(env_.root()), sorted({UTF::path_from_u32(U"a_가.txt")}));
    EXPECT_EQ(session_.undo_rename({})->succeeded, 1u);
    EXPECT_EQ(names_in(env_.root()), sorted({UTF::path_from_u32(decomposed)}));
}

TEST_F(SESSION_REAL_FS_TEST, changes_the_case_of_a_folder_with_its_items)
{
    env_.make_folder(at("f"));
    env_.make_file(at("f/x.txt"), 1);
    session_.add_items({at("f"), at("f/x.txt")}, ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_.items().order();
    session_.direct_edit(ids[0], U"F");
    session_.direct_edit(ids[1], U"y.txt");
    EXPECT_EQ(session_.rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({"F"}));
    EXPECT_EQ(names_in(at("F")), sorted({"y.txt"}));
    EXPECT_EQ(session_.items().find(ids[1])->path, at("F/y.txt"));

    EXPECT_EQ(session_.undo_rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({"f"}));
    EXPECT_EQ(names_in(at("f")), sorted({"x.txt"}));
}

TEST_F(SESSION_REAL_FS_TEST, renames_decomposed_names)
{
    const std::u32string decomposed = U"e\u0301.txt";
    session_.add_items(files({decomposed}), ADD_MODE::as_items);
    const uint64_t id = session_.items().order()[0];
    session_.direct_edit(id, U"\u00E9.txt");
    EXPECT_EQ(session_.preview().changed, 0u);
    session_.direct_edit(id, U"\u00C9.txt");
    EXPECT_EQ(session_.rename({})->succeeded, 1u);
    EXPECT_EQ(names_in(env_.root()), sorted({UTF::path_from_u32(U"\u00C9.txt")}));
    EXPECT_EQ(session_.undo_rename({})->succeeded, 1u);
    EXPECT_EQ(names_in(env_.root()), sorted({UTF::path_from_u32(decomposed)}));
}

TEST_F(SESSION_REAL_FS_TEST, finds_conflicts_with_decomposed_names)
{
    files({U"e\u0301.txt"});
    session_.add_items(files({U"a.txt"}), ADD_MODE::as_items);
    session_.direct_edit(session_.items().order()[0], U"\u00C9.TXT");
    const PREVIEW preview = session_.preview();
    ASSERT_EQ(preview.rows.size(), size_t{1});
    EXPECT_EQ(preview.rows[0].conflict, CONFLICT_KIND::existing_item);
    EXPECT_FALSE(session_.rename({}).has_value());
}

TEST_F(SESSION_REAL_FS_TEST, avoids_existing_temporary_names)
{
    env_.make_file(at(".limenamer-tmp-1"), 9);
    session_.add_items(files({U"a.txt", U"b.txt"}), ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_.items().order();
    session_.direct_edit(ids[0], U"b.txt");
    session_.direct_edit(ids[1], U"a.txt");
    EXPECT_EQ(session_.rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({".limenamer-tmp-1", "a.txt", "b.txt"}));
    EXPECT_EQ(size_of(at(".limenamer-tmp-1")), 9);
    EXPECT_EQ(size_of(at("a.txt")), 2);
}

TEST_F(SESSION_REAL_FS_TEST, reverts_all_after_a_failure)
{
    session_.add_items(files({U"a.txt", U"b.txt", U"c.txt"}), ADD_MODE::as_items);
    session_.add_mod(mod(PREFIX));
    const FILE_LOCK lock(at("b.txt"));
    const std::optional<RENAME_REPORT> report = session_.rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->succeeded, 2u);
    EXPECT_EQ(report->failed, 1u);
    EXPECT_TRUE(report->needs_choice);
    ASSERT_EQ(report->problems.size(), size_t{1});
#if defined(_WIN32)
    EXPECT_EQ(report->problems[0].failure, RENAME_FAILURE::in_use);
#else
    EXPECT_EQ(report->problems[0].failure, RENAME_FAILURE::access_denied);
#endif
    session_.resolve_rename(RESOLVE_CHOICE::revert_all, {});
    EXPECT_EQ(names_in(env_.root()), sorted({"a.txt", "b.txt", "c.txt"}));
    EXPECT_EQ(item_names(), std::vector<std::u32string>({U"a.txt", U"b.txt", U"c.txt"}));
    EXPECT_TRUE(session_.record().empty());
}

TEST_F(SESSION_REAL_FS_TEST, keeps_succeeded_after_a_failure)
{
    session_.add_items(files({U"a.txt", U"b.txt", U"c.txt"}), ADD_MODE::as_items);
    session_.add_mod(mod(PREFIX));
    {
        const FILE_LOCK lock(at("b.txt"));
        ASSERT_TRUE(session_.rename({})->needs_choice);
        session_.resolve_rename(RESOLVE_CHOICE::keep_succeeded, {});
    }
    EXPECT_EQ(names_in(env_.root()), sorted({"a_a.txt", "a_c.txt", "b.txt"}));
    EXPECT_EQ(item_names(), std::vector<std::u32string>({U"a_a.txt", U"b.txt", U"a_c.txt"}));
    EXPECT_EQ(session_.record().size(), size_t{2});
    EXPECT_EQ(session_.undo_rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({"a.txt", "b.txt", "c.txt"}));
}

TEST_F(SESSION_REAL_FS_TEST, skips_files_changed_outside_before_undo)
{
    session_.add_items(files({U"a.txt", U"b.txt", U"c.txt"}), ADD_MODE::as_items);
    session_.add_mod(mod(PREFIX));
    session_.rename({});
    std::filesystem::remove(at("a_a.txt"));
    std::filesystem::remove(at("a_b.txt"));
    env_.make_file(at("a_b.txt"), 2);
    const RENAME_REPORT check = session_.undo_check();
    EXPECT_EQ(check.skipped, 2u);
    ASSERT_EQ(check.problems.size(), size_t{2});
    EXPECT_EQ(check.problems[0].failure, RENAME_FAILURE::not_found);
    EXPECT_EQ(check.problems[1].failure, RENAME_FAILURE::replaced);

    const std::optional<RENAME_REPORT> report = session_.undo_rename({});
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->succeeded, 1u);
    EXPECT_EQ(report->skipped, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({"a_b.txt", "c.txt"}));
    EXPECT_EQ(session_.record().size(), size_t{2});
}

TEST_F(SESSION_REAL_FS_TEST, renames_in_long_paths)
{
    std::filesystem::path folder = env_.root();
    while (folder.native().size() < 300) {
        folder /= std::string(40, 'd');
        env_.make_folder(extended(folder));
    }
    env_.make_file(extended(folder / "aaaa.txt"), 1);
    session_.add_items({folder / "aaaa.txt"}, ADD_MODE::as_items);
    const uint64_t id = session_.items().order()[0];
    session_.direct_edit(id, U"bbbbb.txt");
#if defined(_WIN32)
    EXPECT_FALSE(session_.rename({}).has_value());
#else
    EXPECT_TRUE(session_.rename({}).has_value());
    EXPECT_EQ(session_.undo_rename({})->succeeded, 1u);
#endif
    session_.direct_edit(id, U"bbbb.txt");
    EXPECT_EQ(session_.rename({})->succeeded, 1u);
    EXPECT_EQ(names_in(folder), sorted({"bbbb.txt"}));
    EXPECT_EQ(session_.undo_rename({})->succeeded, 1u);
    EXPECT_EQ(names_in(folder), sorted({"aaaa.txt"}));
}

TEST_F(SESSION_REAL_FS_TEST, renames_a_thousand_files)
{
    std::vector<std::filesystem::path> paths;
    for (int index = 0; index < 1000; ++index) {
        const std::filesystem::path path = at("IMG_" + std::to_string(index) + ".JPG");
        env_.make_file(path, 1);
        paths.push_back(path);
    }
    session_.add_items(paths, ADD_MODE::as_items);
    session_.add_mod(mod(PREFIX));
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    EXPECT_EQ(session_.rename({})->succeeded, 1000u);
    const std::chrono::steady_clock::time_point renamed = std::chrono::steady_clock::now();
    EXPECT_EQ(session_.undo_rename({})->succeeded, 1000u);
    const std::chrono::steady_clock::time_point undone = std::chrono::steady_clock::now();
    EXPECT_EQ(names_in(env_.root()).size(), size_t{1000});
    EXPECT_EQ(size_of(at("IMG_0.JPG")), 1);

    const auto ms = [](auto from, auto to) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(to - from).count();
    };
    RecordProperty("rename_ms", std::to_string(ms(start, renamed)));
    RecordProperty("undo_ms", std::to_string(ms(renamed, undone)));
    std::cout << "rename: " << ms(start, renamed) << " ms, undo: " << ms(renamed, undone)
              << " ms\n";
}
