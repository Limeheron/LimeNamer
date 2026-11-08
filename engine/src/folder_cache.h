#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "file_system.h"
#include "folder_names.h"
#include "text_service.h"
#include "utf.h"

class FOLDER_CACHE
{
public:
    FOLDER_CACHE(const FILE_SYSTEM &file_system, const TEXT_SERVICE &text_service);

    void load(const std::filesystem::path &folder);
    void reload(const std::filesystem::path &folder);
    void refresh();
    void retain(const std::vector<std::filesystem::path> &folders);

    bool has_name(const std::filesystem::path &folder, std::u32string_view name) const;
    const FOLDER_NAMES *find(const std::filesystem::path &folder) const;
    size_t size() const;

private:
    std::optional<std::u32string> key_of(const std::filesystem::path &folder) const;
    FOLDER_NAMES read(const std::filesystem::path &folder) const;

    const FILE_SYSTEM &file_system_;
    const TEXT_SERVICE &text_service_;
    std::unordered_map<std::u32string, FOLDER_NAMES> folders_;
};
