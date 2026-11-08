#include "win_file_system_test.h"

namespace
{
std::filesystem::path prefixed(const std::filesystem::path &path)
{
    return std::filesystem::path(L"\\\\?\\" + path.native());
}
}

TEST(WIN_FILE_SYSTEM, reads_times)
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

TEST(WIN_FILE_SYSTEM, handles_long_paths)
{
    REAL_FS_ENV env;
    std::filesystem::path folder = env.root();
    while (folder.native().size() < 300) {
        folder /= std::wstring(40, L'd');
        env.make_folder(prefixed(folder));
    }
    const std::filesystem::path file = folder / "a.txt";
    env.make_file(prefixed(file), 3);
    FILE_INFO info;
    ASSERT_TRUE(env.file_system().get_info(file, info).ok());
    EXPECT_EQ(info.size, 3);
    std::vector<std::filesystem::path> names;
    ASSERT_TRUE(env.file_system().list_names(folder, names).ok());
    const std::vector<std::filesystem::path> expected = {"a.txt"};
    EXPECT_EQ(names, expected);
    EXPECT_TRUE(env.file_system().rename(file, folder / "b.txt").ok());
    EXPECT_TRUE(env.file_system().get_info(folder / "b.txt", info).ok());
}

TEST(WIN_FILE_SYSTEM, reports_file_in_use)
{
    REAL_FS_ENV env;
    const std::filesystem::path file = env.root() / "a.txt";
    env.make_file(file, 1);
    const HANDLE handle = CreateFileW(file.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    ASSERT_NE(handle, INVALID_HANDLE_VALUE);
    const FS_STATUS status = env.file_system().rename(file, env.root() / "b.txt");
    CloseHandle(handle);
    EXPECT_EQ(status.error, FS_ERROR::in_use);
    FILE_INFO info;
    EXPECT_TRUE(env.file_system().get_info(file, info).ok());
}

TEST(WIN_FILE_SYSTEM, allows_case_only_rename)
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

TEST(WIN_FILE_SYSTEM, handles_symlink_itself)
{
    REAL_FS_ENV env;
    env.make_folder(env.root() / "target");
    std::error_code error;
    std::filesystem::create_directory_symlink(env.root() / "target", env.root() / "link", error);
    if (error) {
        GTEST_SKIP() << "symbolic links are not permitted";
    }
    FILE_INFO info;
    ASSERT_TRUE(env.file_system().get_info(env.root() / "link", info).ok());
    ASSERT_TRUE(env.file_system().rename(env.root() / "link", env.root() / "renamed").ok());
    EXPECT_TRUE(std::filesystem::is_symlink(env.root() / "renamed"));
    EXPECT_TRUE(std::filesystem::is_directory(env.root() / "target"));
}
