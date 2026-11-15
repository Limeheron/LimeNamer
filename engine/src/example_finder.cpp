#include "example_finder.h"

EXAMPLE_FINDER::EXAMPLE_FINDER(const PREVIEW_BUILDER &builder, const NAME_CHECKER &checker)
    : builder_(builder),
      checker_(checker)
{
}

std::optional<EXAMPLE> EXAMPLE_FINDER::find(const ITEM_STORE &items,
    const std::vector<MOD_ENTRY> &mods, const MODIFICATION &draft, uint64_t before_mod_id) const
{
    const std::optional<std::vector<const MODIFICATION *>> previous =
        mods_before(mods, before_mod_id);
    if (!previous) {
        return std::nullopt;
    }
    if (items.size() == 0) {
        return default_example(items, *previous, draft);
    }

    const std::vector<MOD_PLAN> plans = builder_.plan(items, *previous);
    const std::vector<MOD_PLAN> draft_plan = builder_.plan(items, {&draft});
    const std::vector<uint64_t> &order = items.order();
    std::optional<EXAMPLE> first;
    for (size_t index = 0; index < order.size(); ++index) {
        const ITEM &item = *items.find(order[index]);
        if (draft.scope == MOD_SCOPE::items && !draft_plan[0].indexes.contains(item.id)) {
            continue;
        }
        const uint32_t position = static_cast<uint32_t>(index + 1);
        std::u32string before = builder_.apply(item, index, plans);
        if (draft.part == MOD_PART::ext && item.info.is_dir) {
            if (!first) {
                std::u32string same = before;
                first = make(item, position, std::move(before), std::move(same),
                    EXAMPLE_STATE::skipped_dir);
            }
            continue;
        }
        std::u32string after = builder_.apply_from(item, before, index, draft_plan);
        if (after != before) {
            return make(item, position, std::move(before), std::move(after),
                EXAMPLE_STATE::changed);
        }
        if (!first) {
            first = make(item, position, std::move(before), std::move(after),
                EXAMPLE_STATE::unchanged);
        }
    }
    if (first) {
        return first;
    }
    return EXAMPLE();
}

std::optional<std::vector<const MODIFICATION *>> EXAMPLE_FINDER::mods_before(
    const std::vector<MOD_ENTRY> &mods, uint64_t before_mod_id)
{
    std::vector<const MODIFICATION *> previous;
    for (const MOD_ENTRY &entry : mods) {
        if (entry.id == before_mod_id) {
            return previous;
        }
        if (entry.enabled) {
            previous.push_back(&entry.mod);
        }
    }
    if (before_mod_id != 0) {
        return std::nullopt;
    }
    return previous;
}

EXAMPLE EXAMPLE_FINDER::make(const ITEM &item, uint32_t index, std::u32string before,
    std::u32string after, EXAMPLE_STATE state) const
{
    EXAMPLE example;
    example.item_id = item.id;
    example.index = index;
    if (state == EXAMPLE_STATE::changed) {
        const size_t units = NAME_CHECKER::folder_units(
            UTF::path_to_u32(item.folder).value_or(std::u32string()));
        example.errors = checker_.check(after, before, units).errors;
    }
    const std::pair<uint32_t, uint32_t> lengths = PREVIEW_BUILDER::common_lengths(before, after);
    example.prefix_len = lengths.first;
    example.suffix_len = lengths.second;
    example.before = std::move(before);
    example.after = std::move(after);
    example.state = state;
    return example;
}

EXAMPLE EXAMPLE_FINDER::sample(const ITEM_STORE &items, const MODIFICATION &draft) const
{
    return default_example(items, {}, draft);
}

EXAMPLE EXAMPLE_FINDER::default_example(const ITEM_STORE &items,
    const std::vector<const MODIFICATION *> &previous, const MODIFICATION &draft) const
{
    ITEM item;
    item.name = DEFAULT_NAME;
    std::vector<const MODIFICATION *> shared;
    for (const MODIFICATION *mod : previous) {
        if (mod->scope == MOD_SCOPE::all) {
            shared.push_back(mod);
        }
    }
    const std::u32string before = builder_.apply(item, 0, builder_.plan(items, shared));
    MODIFICATION open = draft;
    open.scope = MOD_SCOPE::all;
    const std::vector<MOD_PLAN> draft_plan = builder_.plan(items, {&open});
    const std::u32string after = builder_.apply_from(item, before, 0, draft_plan);
    EXAMPLE example = make(item, 0, before, after,
        after != before ? EXAMPLE_STATE::changed : EXAMPLE_STATE::unchanged);
    example.errors = after != before ? checker_.check(after, before, 0).errors : 0;
    return example;
}
