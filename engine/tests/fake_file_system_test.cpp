#include "fake_file_system_test.h"

TEST(FAKE_FILE_SYSTEM, fail_next_rename_fails_once)
{
    FAKE_FS_ENV env;
    env.make_file(env.root() / "a.txt", 1);
    env.fake().fail_next_rename(env.root() / "A.TXT", FS_ERROR::in_use);
    EXPECT_EQ(env.file_system().rename(env.root() / "a.txt", env.root() / "b.txt").error,
        FS_ERROR::in_use);
    FILE_INFO info;
    EXPECT_TRUE(env.file_system().get_info(env.root() / "a.txt", info).ok());
    EXPECT_TRUE(env.file_system().rename(env.root() / "a.txt", env.root() / "b.txt").ok());
}

TEST(FAKE_FILE_SYSTEM, add_requires_parent_folder)
{
    FAKE_FS_ENV env;
    env.make_file(env.root() / "a.txt", 1);
    EXPECT_THROW(env.make_file(env.root() / "missing/b.txt", 1), std::logic_error);
    EXPECT_THROW(env.make_file(env.root() / "a.txt/b.txt", 1), std::logic_error);
    EXPECT_THROW(env.make_file(env.root() / "A.TXT", 1), std::logic_error);
}

TEST(FAKE_FILE_SYSTEM, children_follow_stored_parent_case)
{
    FAKE_FS_ENV env;
    env.make_folder(env.root() / "Folder");
    env.make_file(env.root() / "folder/a.txt", 1);
    ASSERT_TRUE(env.file_system().rename(env.root() / "FOLDER", env.root() / "Renamed").ok());
    FILE_INFO info;
    EXPECT_TRUE(env.file_system().get_info(env.root() / "Renamed/a.txt", info).ok());
}
