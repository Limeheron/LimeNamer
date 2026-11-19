#include "mac_session_test.h"

namespace
{
const char *const PREFIX =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a_"}]})";
}

TEST_F(MAC_SESSION_TEST, limits_names_to_255_bytes)
{
    session_.add_items(files({U"a.txt", U"b.txt"}), ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_.items().order();
    session_.direct_edit(ids[0], std::u32string(84, U'가') + U".txt");
    EXPECT_EQ(session_.preview().error, 1u);
    EXPECT_FALSE(session_.rename({}).has_value());

    session_.direct_edit(ids[0], std::u32string(83, U'가') + U".txt");
    session_.direct_edit(ids[1], std::u32string(251, U'b') + U".txt");
    EXPECT_EQ(session_.rename({})->succeeded, 2u);
    EXPECT_EQ(session_.undo_rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(env_.root()), sorted({"a.txt", "b.txt"}));
}

TEST_F(MAC_SESSION_TEST, renames_symlinks_themselves)
{
    env_.make_folder(at("d"));
    env_.make_folder(at("d/sub"));
    env_.make_file(at("d/real.txt"), 5);
    std::filesystem::create_symlink(at("d/real.txt"), at("d/link_file"));
    std::filesystem::create_directory_symlink(at("d/sub"), at("d/link_dir"));
    std::filesystem::create_symlink(at("d/missing"), at("d/broken"));
    EXPECT_EQ(session_.add_items({at("d")}, ADD_MODE::folder_contents).added, 4u);
    session_.add_mod(mod(PREFIX));
    EXPECT_EQ(session_.rename({})->succeeded, 4u);
    EXPECT_EQ(names_in(at("d")),
        sorted({"a_broken", "a_link_dir", "a_link_file", "a_real.txt", "sub"}));
    EXPECT_EQ(std::filesystem::read_symlink(at("d/a_link_dir")), at("d/sub"));
    EXPECT_EQ(size_of(at("d/a_real.txt")), 5);
}

TEST_F(MAC_SESSION_TEST, treats_packages_as_files)
{
    env_.make_folder(at("d"));
    env_.make_folder(at("d/Notes.rtfd"));
    env_.make_file(at("d/Notes.rtfd/TXT.rtf"), 1);
    env_.make_folder(at("d/plain"));
    EXPECT_EQ(session_.add_items({at("d")}, ADD_MODE::folder_contents).added, 1u);
    session_.add_items({at("d/plain")}, ADD_MODE::as_items);
    session_.add_mod(mod(R"({"kind":"insert","position":{"type":"end"},
        "value":[{"text":"_1"}]})"));
    const PREVIEW preview = session_.preview();
    ASSERT_EQ(preview.rows.size(), size_t{2});
    EXPECT_EQ(preview.rows[0].new_name, U"Notes_1.rtfd");
    EXPECT_FALSE(preview.rows[0].is_dir);
    EXPECT_EQ(preview.rows[1].new_name, U"plain_1");
    EXPECT_TRUE(preview.rows[1].is_dir);
    EXPECT_EQ(session_.rename({})->succeeded, 2u);
    EXPECT_EQ(names_in(at("d/Notes_1.rtfd")), sorted({"TXT.rtf"}));
}

TEST_F(MAC_SESSION_TEST, reports_folders_without_write_access)
{
    env_.make_folder(at("d"));
    env_.make_file(at("d/a.txt"), 1);
    session_.add_items({at("d/a.txt")}, ADD_MODE::as_items);
    session_.add_mod(mod(PREFIX));
    ASSERT_EQ(chmod(at("d").c_str(), 0555), 0);
    const std::optional<RENAME_REPORT> report = session_.rename({});
    chmod(at("d").c_str(), 0755);
    ASSERT_TRUE(report.has_value());
    EXPECT_EQ(report->failed, 1u);
    ASSERT_EQ(report->problems.size(), size_t{1});
    EXPECT_EQ(report->problems[0].failure, RENAME_FAILURE::access_denied);
}

TEST_F(MAC_SESSION_TEST, fails_for_contents_of_unreadable_folders)
{
    env_.make_folder(at("d"));
    env_.make_file(at("d/a.txt"), 1);
    ASSERT_EQ(chmod(at("d").c_str(), 0300), 0);
    const ADD_RESULT result = session_.add_items({at("d")}, ADD_MODE::folder_contents);
    chmod(at("d").c_str(), 0755);
    EXPECT_EQ(result.added, 0u);
    EXPECT_EQ(result.failed, 1u);
}
