#include "preview_builder.h"

PREVIEW_BUILDER::PREVIEW_BUILDER(const TEXT_SERVICE &text_service, const NAME_CHECKER &checker)
    : text_service_(text_service),
      checker_(checker),
      applier_(text_service),
      resolver_(text_service)
{
}

PREVIEW PREVIEW_BUILDER::build(const ITEM_STORE &items, const std::vector<MOD_ENTRY> &mods,
    const OPTIONS &options, const FOLDER_CACHE &cache) const
{
    const std::vector<MOD_PLAN> plans = plan(items, enabled(mods));
    std::unordered_set<uint64_t> direct;
    for (const MOD_ENTRY &entry : mods) {
        if (entry.enabled && entry.mod.direct) {
            direct.insert(entry.mod.targets.begin(), entry.mod.targets.end());
        }
    }

    const std::vector<uint64_t> &order = items.order();
    PREVIEW preview;
    preview.rows.reserve(order.size());
    std::vector<CONFLICT_ENTRY> entries;
    entries.reserve(order.size());
    for (size_t index = 0; index < order.size(); ++index) {
        const ITEM &item = *items.find(order[index]);
        PREVIEW_ROW row;
        row.item_id = item.id;
        row.folder = item.folder;
        row.name = item.name;
        row.new_name = apply(item, index, plans);
        row.size = item.info.size;
        row.modified_ms = item.info.modified_ms;
        row.created_ms = item.info.created_ms;
        row.is_dir = item.info.is_dir;
        row.changed = row.new_name != item.name;
        row.direct = direct.contains(item.id);

        CONFLICT_ENTRY entry;
        entry.id = item.id;
        entry.folder = item.folder;
        entry.current_name = item.name;
        entry.new_name = row.new_name;
        entry.changed = row.changed;
        entry.is_dir = row.is_dir;
        entries.push_back(std::move(entry));
        preview.rows.push_back(std::move(row));
    }

    if (options.duplicate.enabled) {
        const std::vector<std::optional<std::u32string>> adjusted =
            resolver_.adjust(entries, cache, options.duplicate);
        for (size_t index = 0; index < adjusted.size(); ++index) {
            if (adjusted[index]) {
                PREVIEW_ROW &row = preview.rows[index];
                row.new_name = text_service_.to_nfc(*adjusted[index]);
                row.adjusted = true;
                row.changed = row.new_name != row.name;
                entries[index].new_name = row.new_name;
                entries[index].changed = row.changed;
            }
        }
    }
    finish(preview, entries, cache, order);
    return preview;
}

std::vector<MOD_PLAN> PREVIEW_BUILDER::plan(const ITEM_STORE &items,
    const std::vector<const MODIFICATION *> &mods) const
{
    std::vector<MOD_PLAN> plans;
    plans.reserve(mods.size());
    for (const MODIFICATION *mod : mods) {
        MOD_PLAN plan;
        plan.mod = mod;
        if (mod->scope == MOD_SCOPE::items) {
            const std::unordered_set<uint64_t> targets(mod->targets.begin(), mod->targets.end());
            uint64_t next = 0;
            for (const uint64_t id : items.order()) {
                if (targets.contains(id)) {
                    plan.indexes.emplace(id, next++);
                }
            }
        }
        plans.push_back(std::move(plan));
    }
    return plans;
}

std::u32string PREVIEW_BUILDER::apply(const ITEM &item, uint64_t list_index,
    const std::vector<MOD_PLAN> &plans) const
{
    TOKEN_CONTEXT context;
    context.original_name = item.name;
    const std::optional<std::u32string> parent = UTF::path_to_u32(item.folder.filename());
    context.parent_name = parent ? text_service_.to_nfc(*parent) : std::u32string();
    context.is_dir = item.info.is_dir;
    context.size = item.info.size;
    context.modified_ms = item.info.modified_ms;
    context.created_ms = item.info.created_ms;

    std::u32string name = item.name;
    for (const MOD_PLAN &plan : plans) {
        const MODIFICATION &mod = *plan.mod;
        if (mod.scope == MOD_SCOPE::items) {
            const auto found = plan.indexes.find(item.id);
            if (found == plan.indexes.end()) {
                continue;
            }
            context.index = found->second;
        } else {
            context.index = list_index;
        }
        const bool has_value = mod.kind == MOD_KIND::insert || mod.kind == MOD_KIND::replace;
        const std::u32string value = has_value ? TOKEN_FORMATTER::build(mod, context) : U"";
        name = applier_.apply(mod, name, item.info.is_dir, value);
    }
    return text_service_.to_nfc(name);
}

std::vector<const MODIFICATION *> PREVIEW_BUILDER::enabled(const std::vector<MOD_ENTRY> &mods)
{
    std::vector<const MODIFICATION *> result;
    for (const MOD_ENTRY &entry : mods) {
        if (entry.enabled) {
            result.push_back(&entry.mod);
        }
    }
    return result;
}

void PREVIEW_BUILDER::highlight(PREVIEW_ROW &row)
{
    const std::u32string &before = row.name;
    const std::u32string &after = row.new_name;
    size_t prefix = 0;
    while (prefix < before.size() && prefix < after.size() && before[prefix] == after[prefix]) {
        ++prefix;
    }
    size_t suffix = 0;
    while (suffix < before.size() - prefix && suffix < after.size() - prefix
        && before[before.size() - 1 - suffix] == after[after.size() - 1 - suffix]) {
        ++suffix;
    }
    row.prefix_len = static_cast<uint32_t>(prefix);
    row.suffix_len = static_cast<uint32_t>(suffix);
}

void PREVIEW_BUILDER::finish(PREVIEW &preview, const std::vector<CONFLICT_ENTRY> &entries,
    const FOLDER_CACHE &cache, const std::vector<uint64_t> &order) const
{
    const std::vector<CONFLICT> conflicts = resolver_.find(entries, cache);
    std::unordered_map<uint64_t, uint32_t> positions;
    for (size_t index = 0; index < order.size(); ++index) {
        positions.emplace(order[index], static_cast<uint32_t>(index + 1));
    }
    std::unordered_map<std::filesystem::path::string_type, size_t> folder_units;

    for (size_t index = 0; index < preview.rows.size(); ++index) {
        PREVIEW_ROW &row = preview.rows[index];
        if (!row.changed) {
            row.status = ROW_STATUS::unchanged;
            row.prefix_len = static_cast<uint32_t>(row.name.size());
            continue;
        }
        ++preview.changed;
        const auto known = folder_units.find(row.folder.native());
        size_t units = 0;
        if (known != folder_units.end()) {
            units = known->second;
        } else {
            units = NAME_CHECKER::folder_units(
                UTF::path_to_u32(row.folder).value_or(std::u32string()));
            folder_units.emplace(row.folder.native(), units);
        }
        const NAME_CHECK check = checker_.check(row.new_name, row.name, units);
        row.errors = check.errors;
        row.warnings = check.warnings;
        row.conflict = conflicts[index].kind;
        if (conflicts[index].kind == CONFLICT_KIND::list_item) {
            row.conflict_index = positions.at(conflicts[index].other_id);
        }
        if (row.errors != 0) {
            row.status = ROW_STATUS::error;
            ++preview.error;
        } else if (row.conflict != CONFLICT_KIND::none) {
            row.status = ROW_STATUS::conflict;
            ++preview.conflict;
        } else if (row.warnings != 0) {
            row.status = ROW_STATUS::warning;
            ++preview.warning;
        } else {
            row.status = ROW_STATUS::ok;
        }
        if (row.errors == 0 && row.conflict == CONFLICT_KIND::none) {
            ++preview.runnable;
        }
        highlight(row);
    }
}
