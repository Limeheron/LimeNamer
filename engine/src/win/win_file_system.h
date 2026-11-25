#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <windows.h>

#include "file_system.h"

class WIN_FILE_SYSTEM : public FILE_SYSTEM
{
public:
    FS_STATUS get_info(const std::filesystem::path &path, FILE_INFO &info) const override;
    FS_STATUS list_names(const std::filesystem::path &folder,
        std::vector<std::filesystem::path> &names) const override;
    FS_STATUS list_entries(const std::filesystem::path &folder,
        std::vector<FOLDER_ENTRY> &entries) const override;
    FS_STATUS rename(const std::filesystem::path &from,
        const std::filesystem::path &to) override;

private:
    using SCOPED_HANDLE = std::unique_ptr<void, decltype(&CloseHandle)>;
    using SCOPED_FIND = std::unique_ptr<void, decltype(&FindClose)>;

    static constexpr const int64_t EPOCH_TICKS = 116444736000000000LL;
    static constexpr const size_t ENTRY_BUFFER_WORDS = 8192;

    static bool read_entries(const std::filesystem::path &folder,
        std::vector<FOLDER_ENTRY> &entries);
    static FOLDER_ENTRY to_entry(const FILE_ID_EXTD_DIR_INFO &source, std::wstring_view name,
        uint64_t volume);
    static std::wstring extended(const std::filesystem::path &path);
    static bool is_directory(const std::wstring &path);
    static FS_STATUS from_error(DWORD error);
    static int64_t to_ms(const FILETIME &time);
    static int64_t to_ms(int64_t ticks);
};
