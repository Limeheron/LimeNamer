#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "conflict.h"
#include "conflict_entry.h"
#include "conflict_keys.h"
#include "duplicate_options.h"
#include "folder_cache.h"
#include "folder_names.h"
#include "name_parts.h"
#include "text_service.h"

class CONFLICT_RESOLVER
{
public:
    explicit CONFLICT_RESOLVER(const TEXT_SERVICE &text_service);

    std::vector<CONFLICT> find(const std::vector<CONFLICT_ENTRY> &entries,
        const FOLDER_CACHE &cache) const;
    std::vector<std::optional<std::u32string>> adjust(const std::vector<CONFLICT_ENTRY> &entries,
        const FOLDER_CACHE &cache, const DUPLICATE_OPTIONS &options) const;

private:
    static constexpr const uint64_t MAX_NUMBER = 1000000;

    static std::u32string numbered(std::u32string_view name, bool is_dir, uint64_t number,
        const DUPLICATE_OPTIONS &options);

    CONFLICT_KEYS prepare(const std::vector<CONFLICT_ENTRY> &entries,
        const FOLDER_CACHE &cache) const;

    const TEXT_SERVICE &text_service_;
};
