#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "conflict_entry.h"
#include "conflict_resolver.h"
#include "file_info.h"
#include "folder_cache.h"
#include "item.h"
#include "item_store.h"
#include "mod_applier.h"
#include "mod_entry.h"
#include "mod_plan.h"
#include "name_checker.h"
#include "options.h"
#include "preview.h"
#include "text_service.h"
#include "token_context.h"
#include "token_formatter.h"
#include "utf.h"

class PREVIEW_BUILDER
{
public:
    PREVIEW_BUILDER(const TEXT_SERVICE &text_service, const NAME_CHECKER &checker);

    PREVIEW build(const ITEM_STORE &items, const std::vector<MOD_ENTRY> &mods,
        const OPTIONS &options, const FOLDER_CACHE &cache) const;

    std::vector<MOD_PLAN> plan(const ITEM_STORE &items,
        const std::vector<const MODIFICATION *> &mods) const;
    std::u32string apply(const ITEM &item, uint64_t list_index,
        const std::vector<MOD_PLAN> &plans) const;
    std::u32string apply_from(const ITEM &item, std::u32string name, uint64_t list_index,
        const std::vector<MOD_PLAN> &plans) const;

    static std::vector<const MODIFICATION *> enabled(const std::vector<MOD_ENTRY> &mods);
    static bool is_folder(const FILE_INFO &info);
    static int64_t size_of(const FILE_INFO &info);
    static std::pair<uint32_t, uint32_t> common_lengths(std::u32string_view before,
        std::u32string_view after);

private:
    static void highlight(PREVIEW_ROW &row);

    void finish(PREVIEW &preview, const std::vector<CONFLICT_ENTRY> &entries,
        const FOLDER_CACHE &cache, const std::vector<uint64_t> &order) const;

    const TEXT_SERVICE &text_service_;
    const NAME_CHECKER &checker_;
    MOD_APPLIER applier_;
    CONFLICT_RESOLVER resolver_;
};
