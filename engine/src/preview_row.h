#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "conflict_kind.h"
#include "row_status.h"

struct PREVIEW_ROW
{
    uint64_t item_id = 0;
    std::filesystem::path folder;
    std::u32string name;
    std::u32string new_name;
    int64_t size = 0;
    int64_t modified_ms = 0;
    int64_t created_ms = 0;
    uint32_t prefix_len = 0;
    uint32_t suffix_len = 0;
    uint32_t errors = 0;
    uint32_t warnings = 0;
    ROW_STATUS status = ROW_STATUS::unchanged;
    CONFLICT_KIND conflict = CONFLICT_KIND::none;
    uint32_t conflict_index = 0;
    bool is_dir = false;
    bool changed = false;
    bool adjusted = false;
    bool direct = false;
};
