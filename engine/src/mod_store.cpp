#include "mod_store.h"

MOD_STORE::MOD_STORE(const TEXT_SERVICE &text_service)
    : text_service_(text_service)
{
}

uint64_t MOD_STORE::add(MODIFICATION mod)
{
    normalize(mod);
    MOD_ENTRY entry;
    entry.id = next_id_++;
    entry.mod = std::move(mod);
    entries_.push_back(std::move(entry));
    return entries_.back().id;
}

bool MOD_STORE::update(uint64_t id, MODIFICATION mod)
{
    const std::optional<size_t> index = index_of(id);
    if (!index) {
        return false;
    }
    MOD_ENTRY &entry = entries_[*index];
    if (mod.kind != entry.mod.kind) {
        throw std::invalid_argument("modification kind cannot change");
    }
    normalize(mod);
    mod.direct = entry.mod.direct;
    entry.mod = std::move(mod);
    return true;
}

bool MOD_STORE::remove(uint64_t id)
{
    const std::optional<size_t> index = index_of(id);
    if (!index) {
        return false;
    }
    entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(*index));
    return true;
}

std::optional<bool> MOD_STORE::move(uint64_t id, uint64_t target_id, bool after, bool dry_run)
{
    const std::optional<size_t> from = index_of(id);
    const std::optional<size_t> target = index_of(target_id);
    if (!from || !target) {
        return std::nullopt;
    }
    size_t to = *target + (after ? 1 : 0);
    if (to > *from) {
        --to;
    }
    if (to == *from) {
        return false;
    }
    if (!dry_run) {
        MOD_ENTRY entry = std::move(entries_[*from]);
        entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(*from));
        entries_.insert(entries_.begin() + static_cast<std::ptrdiff_t>(to), std::move(entry));
    }
    return true;
}

bool MOD_STORE::set_enabled(uint64_t id, bool enabled)
{
    const std::optional<size_t> index = index_of(id);
    if (!index) {
        return false;
    }
    entries_[*index].enabled = enabled;
    return true;
}

void MOD_STORE::set_all_enabled(bool enabled)
{
    for (MOD_ENTRY &entry : entries_) {
        entry.enabled = enabled;
    }
}

uint64_t MOD_STORE::direct_edit(uint64_t item_id, std::u32string new_name)
{
    MODIFICATION mod;
    mod.kind = MOD_KIND::replace;
    mod.part = MOD_PART::full;
    mod.position.type = POSITION_TYPE::all;
    mod.value.emplace_back(std::move(new_name));
    mod.scope = MOD_SCOPE::items;
    mod.targets = {item_id};
    mod.direct = true;
    normalize(mod);
    for (MOD_ENTRY &entry : entries_) {
        if (entry.mod.direct && entry.mod.targets == mod.targets) {
            entry.mod = std::move(mod);
            entry.enabled = true;
            return entry.id;
        }
    }
    return add(std::move(mod));
}

const std::vector<MOD_ENTRY> &MOD_STORE::entries() const
{
    return entries_;
}

const MOD_ENTRY *MOD_STORE::find(uint64_t id) const
{
    const std::optional<size_t> index = index_of(id);
    return index ? &entries_[*index] : nullptr;
}

std::optional<size_t> MOD_STORE::index_of(uint64_t id) const
{
    for (size_t index = 0; index < entries_.size(); ++index) {
        if (entries_[index].id == id) {
            return index;
        }
    }
    return std::nullopt;
}

void MOD_STORE::normalize(MODIFICATION &mod) const
{
    mod.position.text = text_service_.to_nfc(mod.position.text);
    mod.position.start_text = text_service_.to_nfc(mod.position.start_text);
    mod.position.end_text = text_service_.to_nfc(mod.position.end_text);
    for (VALUE_PIECE &piece : mod.value) {
        if (std::u32string *text = std::get_if<std::u32string>(&piece)) {
            *text = text_service_.to_nfc(*text);
        }
    }
}
