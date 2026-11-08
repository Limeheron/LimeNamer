#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "folder_cache.h"
#include "folder_names.h"
#include "rename_request.h"
#include "rename_step.h"
#include "text_service.h"
#include "utf.h"

class RENAME_PLANNER
{
public:
    static constexpr const char32_t TEMP_PREFIX[] = U".limenamer-tmp-";

    explicit RENAME_PLANNER(const TEXT_SERVICE &text_service);

    std::vector<RENAME_STEP> plan(const std::vector<RENAME_REQUEST> &requests,
        const FOLDER_CACHE &cache) const;

private:
    void plan_folder(const std::vector<const RENAME_REQUEST *> &requests,
        const FOLDER_NAMES *existing, std::vector<RENAME_STEP> &steps) const;

    const TEXT_SERVICE &text_service_;
};
