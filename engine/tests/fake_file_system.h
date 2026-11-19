#pragma once

#include <cstdint>
#include <deque>
#include <filesystem>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "fake_entry.h"
#include "file_system.h"
#include "text_service.h"
#include "utf.h"

class FAKE_FILE_SYSTEM : public FILE_SYSTEM
{
public:
    explicit FAKE_FILE_SYSTEM(const TEXT_SERVICE &text_service);

    void add_folder(const std::filesystem::path &path);
    void add_package(const std::filesystem::path &path);
    void add_file(const std::filesystem::path &path, int64_t size, bool hidden = false);
    void fail_next_rename(const std::filesystem::path &from, FS_ERROR error);
    void set_modified(const std::filesystem::path &path, int64_t modified_ms);
    void remove(const std::filesystem::path &path);

    FS_STATUS get_info(const std::filesystem::path &path, FILE_INFO &info) const override;
    FS_STATUS list_names(const std::filesystem::path &folder,
        std::vector<std::filesystem::path> &names) const override;
    FS_STATUS rename(const std::filesystem::path &from,
        const std::filesystem::path &to) override;

private:
    static constexpr const char32_t SEPARATOR =
        static_cast<char32_t>(std::filesystem::path::preferred_separator);

    std::u32string key_of(const std::filesystem::path &path) const;
    void add_entry(const std::filesystem::path &path, bool is_dir, int64_t size, bool hidden);

    const TEXT_SERVICE &text_service_;
    std::map<std::u32string, FAKE_ENTRY> entries_;
    std::map<std::u32string, std::deque<FS_ERROR>> rename_failures_;
    uint64_t next_id_ = 1;
};
