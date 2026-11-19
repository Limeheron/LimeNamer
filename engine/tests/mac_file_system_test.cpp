#include "mac_file_system_test.h"

TEST(MAC_FILE_SYSTEM, reads_times)
{
    REAL_FS_ENV env;
    env.make_file(env.root() / "a.txt", 1);
    FILE_INFO info;
    ASSERT_TRUE(env.file_system().get_info(env.root() / "a.txt", info).ok());
    const std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    const double tolerance = 60000.0;
    EXPECT_NEAR(static_cast<double>(info.modified_ms), static_cast<double>(now.count()),
        tolerance);
    EXPECT_NEAR(static_cast<double>(info.created_ms), static_cast<double>(now.count()),
        tolerance);
}

TEST(MAC_FILE_SYSTEM, handles_symlink_itself)
{
    REAL_FS_ENV env;
    env.make_folder(env.root() / "target");
    std::filesystem::create_directory_symlink(env.root() / "target", env.root() / "link");
    FILE_INFO info;
    ASSERT_TRUE(env.file_system().get_info(env.root() / "link", info).ok());
    EXPECT_FALSE(info.is_dir);
    ASSERT_TRUE(env.file_system().rename(env.root() / "link", env.root() / "renamed").ok());
    EXPECT_TRUE(std::filesystem::is_symlink(env.root() / "renamed"));
    EXPECT_TRUE(std::filesystem::is_directory(env.root() / "target"));
}

TEST(MAC_FILE_SYSTEM, keeps_stored_normalization)
{
    REAL_FS_ENV env;
    const std::string nfd = "e\xCC\x81.txt";
    env.make_file(env.root() / nfd, 1);
    std::vector<std::filesystem::path> names;
    ASSERT_TRUE(env.file_system().list_names(env.root(), names).ok());
    const std::vector<std::filesystem::path> expected = {nfd};
    EXPECT_EQ(names, expected);
}

TEST(MAC_FILE_SYSTEM, allows_case_only_rename)
{
    REAL_FS_ENV env;
    env.make_file(env.root() / "a.txt", 1);
    EXPECT_EQ(env.file_system().rename(env.root() / "a.txt", env.root() / "A.txt").error,
        FS_ERROR::none);
    std::vector<std::filesystem::path> names;
    ASSERT_TRUE(env.file_system().list_names(env.root(), names).ok());
    const std::vector<std::filesystem::path> expected = {"A.txt"};
    EXPECT_EQ(names, expected);
}

TEST(MAC_FILE_SYSTEM, reads_hidden_flag)
{
    REAL_FS_ENV env;
    env.make_file(env.root() / "shown.txt", 1);
    env.make_file(env.root() / "hidden.txt", 1);
    ASSERT_EQ(chflags((env.root() / "hidden.txt").c_str(), UF_HIDDEN), 0);
    FILE_INFO shown;
    FILE_INFO hidden;
    ASSERT_TRUE(env.file_system().get_info(env.root() / "shown.txt", shown).ok());
    ASSERT_TRUE(env.file_system().get_info(env.root() / "hidden.txt", hidden).ok());
    EXPECT_FALSE(shown.hidden);
    EXPECT_TRUE(hidden.hidden);
}

TEST(MAC_FILE_SYSTEM, reads_package_flag)
{
    REAL_FS_ENV env;
    env.make_folder(env.root() / "Notes.rtfd");
    env.make_folder(env.root() / "plain");
    env.make_file(env.root() / "a.txt", 1);
    FILE_INFO package;
    FILE_INFO folder;
    FILE_INFO file;
    ASSERT_TRUE(env.file_system().get_info(env.root() / "Notes.rtfd", package).ok());
    ASSERT_TRUE(env.file_system().get_info(env.root() / "plain", folder).ok());
    ASSERT_TRUE(env.file_system().get_info(env.root() / "a.txt", file).ok());
    EXPECT_TRUE(package.is_dir);
    EXPECT_TRUE(package.package);
    EXPECT_FALSE(folder.package);
    EXPECT_FALSE(file.package);
}

TEST(MAC_FILE_SYSTEM, renames_after_checking_the_target)
{
    REAL_FS_ENV env;
    env.make_file(env.root() / "a.txt", 1);
    env.make_file(env.root() / "b.txt", 2);
    EXPECT_EQ(MAC_FILE_SYSTEM::rename_checked(env.root() / "a.txt", env.root() / "B.TXT").error,
        FS_ERROR::already_exists);
    EXPECT_EQ(MAC_FILE_SYSTEM::rename_checked(env.root() / "a.txt", env.root() / "A.txt").error,
        FS_ERROR::none);
    EXPECT_EQ(MAC_FILE_SYSTEM::rename_checked(env.root() / "A.txt", env.root() / "c.txt").error,
        FS_ERROR::none);
    EXPECT_EQ(MAC_FILE_SYSTEM::rename_checked(env.root() / "x.txt", env.root() / "d.txt").error,
        FS_ERROR::not_found);
    std::vector<std::filesystem::path> names;
    ASSERT_TRUE(env.file_system().list_names(env.root(), names).ok());
    std::sort(names.begin(), names.end());
    EXPECT_EQ(names, std::vector<std::filesystem::path>({"b.txt", "c.txt"}));
}
