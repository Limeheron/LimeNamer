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
