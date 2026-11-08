#include "real_fs_env.h"

REAL_FS_ENV::REAL_FS_ENV()
    : file_system_(PLATFORM::create_file_system())
{
    std::random_device random;
    const std::filesystem::path base = std::filesystem::temp_directory_path();
    for (int attempt = 0; attempt < 16; ++attempt) {
        const std::filesystem::path candidate =
            base / ("limenamer_test_" + std::to_string(random()) + std::to_string(random()));
        if (std::filesystem::create_directory(candidate)) {
            root_ = candidate;
            return;
        }
    }
    throw std::runtime_error("failed to create test folder");
}

REAL_FS_ENV::~REAL_FS_ENV()
{
    std::error_code error;
#if defined(_WIN32)
    std::filesystem::remove_all(std::filesystem::path(L"\\\\?\\" + root_.native()), error);
#else
    std::filesystem::remove_all(root_, error);
#endif
}

FILE_SYSTEM &REAL_FS_ENV::file_system()
{
    return *file_system_;
}

const std::filesystem::path &REAL_FS_ENV::root() const
{
    return root_;
}

void REAL_FS_ENV::make_file(const std::filesystem::path &path, int64_t size)
{
    std::ofstream out(path, std::ios::binary);
    out << std::string(static_cast<size_t>(size), 'x');
    if (!out) {
        throw std::runtime_error("failed to create test file");
    }
}

void REAL_FS_ENV::make_folder(const std::filesystem::path &path)
{
    if (!std::filesystem::create_directory(path)) {
        throw std::runtime_error("failed to create test folder");
    }
}
