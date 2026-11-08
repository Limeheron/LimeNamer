#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "conflict.h"
#include "conflict_entry.h"
#include "folder_cache.h"
#include "folder_names.h"
#include "text_service.h"

class CONFLICT_RESOLVER
{
public:
    explicit CONFLICT_RESOLVER(const TEXT_SERVICE &text_service);

    std::vector<CONFLICT> find(const std::vector<CONFLICT_ENTRY> &entries,
        const FOLDER_CACHE &cache) const;

private:
    const TEXT_SERVICE &text_service_;
};
