#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "example.h"
#include "item.h"
#include "item_store.h"
#include "mod_entry.h"
#include "mod_plan.h"
#include "modification.h"
#include "name_checker.h"
#include "name_parts.h"
#include "preview_builder.h"
#include "utf.h"

class EXAMPLE_FINDER
{
public:
    static constexpr const char32_t DEFAULT_NAME[] = U"IMG_0001.JPG";

    EXAMPLE_FINDER(const PREVIEW_BUILDER &builder, const NAME_CHECKER &checker);

    std::optional<EXAMPLE> find(const ITEM_STORE &items, const std::vector<MOD_ENTRY> &mods,
        const MODIFICATION &draft, uint64_t before_mod_id) const;

private:
    static std::optional<std::vector<const MODIFICATION *>> mods_before(
        const std::vector<MOD_ENTRY> &mods, uint64_t before_mod_id);

    EXAMPLE make(const ITEM &item, uint32_t index, std::u32string before, std::u32string after,
        EXAMPLE_STATE state) const;
    EXAMPLE default_example(const ITEM_STORE &items,
        const std::vector<const MODIFICATION *> &previous, const MODIFICATION &draft) const;

    const PREVIEW_BUILDER &builder_;
    const NAME_CHECKER &checker_;
};
