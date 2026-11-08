#include "fake_fs_env.h"

FAKE_FS_ENV::FAKE_FS_ENV()
    : text_service_(PLATFORM::create_text_service()),
      file_system_(*text_service_),
      root_("/fake")
{
    file_system_.add_folder(root_);
}

FILE_SYSTEM &FAKE_FS_ENV::file_system()
{
    return file_system_;
}

FAKE_FILE_SYSTEM &FAKE_FS_ENV::fake()
{
    return file_system_;
}

const std::filesystem::path &FAKE_FS_ENV::root() const
{
    return root_;
}

void FAKE_FS_ENV::make_file(const std::filesystem::path &path, int64_t size)
{
    file_system_.add_file(path, size);
}

void FAKE_FS_ENV::make_folder(const std::filesystem::path &path)
{
    file_system_.add_folder(path);
}
