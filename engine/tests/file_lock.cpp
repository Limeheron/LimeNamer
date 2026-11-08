#include "file_lock.h"

FILE_LOCK::FILE_LOCK(const std::filesystem::path &path)
    : path_(path)
{
#if defined(_WIN32)
    handle_ = CreateFileW(path_.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("failed to lock test file");
    }
#else
    if (chflags(path_.c_str(), UF_IMMUTABLE) != 0) {
        throw std::runtime_error("failed to lock test file");
    }
#endif
}

FILE_LOCK::~FILE_LOCK()
{
#if defined(_WIN32)
    CloseHandle(handle_);
#else
    chflags(path_.c_str(), 0);
#endif
}
