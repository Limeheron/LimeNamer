#include "item_store.h"

ITEM_STORE::ITEM_STORE(const FILE_SYSTEM &file_system, const TEXT_SERVICE &text_service)
    : file_system_(file_system),
      text_service_(text_service),
      compare_(text_service)
{
}

ADD_RESULT ITEM_STORE::add(const std::vector<std::filesystem::path> &paths, ADD_MODE mode)
{
    ADD_RESULT result;
    for (const std::filesystem::path &path : paths) {
        if (mode == ADD_MODE::as_items) {
            add_item(path, result);
        } else {
            add_contents(path, result);
        }
    }
    const SORT_STATE added_order;
    if (result.added > 0 && sort_state_ != added_order) {
        sort_state_ = {SORT_KEY::custom, false};
    }
    return result;
}

size_t ITEM_STORE::remove(const std::vector<uint64_t> &ids)
{
    const std::unordered_set<uint64_t> targets(ids.begin(), ids.end());
    std::vector<uint64_t> kept;
    kept.reserve(order_.size());
    size_t removed = 0;
    for (const uint64_t id : order_) {
        if (targets.contains(id)) {
            keys_.erase(items_.at(id).key);
            ++removed;
        } else {
            kept.push_back(id);
        }
    }
    order_ = std::move(kept);
    return removed;
}

void ITEM_STORE::clear()
{
    order_.clear();
    keys_.clear();
    sort_state_ = SORT_STATE();
}

std::optional<bool> ITEM_STORE::move(const std::vector<uint64_t> &ids, uint64_t target_id,
    bool after, bool dry_run)
{
    if (ids.empty() || !contains(target_id)) {
        return std::nullopt;
    }
    std::unordered_set<uint64_t> moving;
    for (const uint64_t id : ids) {
        if (!contains(id)) {
            return std::nullopt;
        }
        moving.insert(id);
    }

    const size_t target_index = static_cast<size_t>(
        std::find(order_.begin(), order_.end(), target_id) - order_.begin());
    const size_t gap = target_index + (after ? 1 : 0);
    std::vector<uint64_t> picked;
    std::vector<uint64_t> rest;
    size_t insert_at = 0;
    for (size_t index = 0; index < order_.size(); ++index) {
        const uint64_t id = order_[index];
        if (moving.contains(id)) {
            picked.push_back(id);
        } else {
            rest.push_back(id);
            if (index < gap) {
                ++insert_at;
            }
        }
    }

    const auto split = rest.begin() + static_cast<std::ptrdiff_t>(insert_at);
    std::vector<uint64_t> next(rest.begin(), split);
    next.insert(next.end(), picked.begin(), picked.end());
    next.insert(next.end(), split, rest.end());
    const bool changed = next != order_;
    if (changed && !dry_run) {
        order_ = std::move(next);
        sort_state_ = {SORT_KEY::custom, false};
    }
    return changed;
}

void ITEM_STORE::sort(SORT_KEY key, bool descending)
{
    if (key == SORT_KEY::custom) {
        throw std::invalid_argument("custom order cannot be sorted");
    }
    std::unordered_map<uint64_t, std::u32string> name_keys;
    if (key != SORT_KEY::added) {
        for (const uint64_t id : order_) {
            name_keys.emplace(id, compare_.key(items_.at(id).name));
        }
    }
    const bool natural = key != SORT_KEY::name_ordinal;
    const auto by_name = [&](uint64_t a, uint64_t b) {
        const int result = NATURAL_COMPARE::compare_keys(name_keys.at(a), name_keys.at(b),
            natural);
        if (result != 0) {
            return result < 0;
        }
        const int exact = items_.at(a).name.compare(items_.at(b).name);
        if (exact != 0) {
            return exact < 0;
        }
        return a < b;
    };

    std::vector<uint64_t> sorted = order_;
    switch (key) {
    case SORT_KEY::added:
        std::sort(sorted.begin(), sorted.end());
        break;
    case SORT_KEY::name_natural:
    case SORT_KEY::name_ordinal:
        std::sort(sorted.begin(), sorted.end(), by_name);
        break;
    case SORT_KEY::modified:
        std::sort(sorted.begin(), sorted.end(), [&](uint64_t a, uint64_t b) {
            const int64_t a_time = items_.at(a).info.modified_ms;
            const int64_t b_time = items_.at(b).info.modified_ms;
            if (a_time != b_time) {
                return a_time < b_time;
            }
            return by_name(a, b);
        });
        break;
    case SORT_KEY::custom:
        break;
    }
    if (descending) {
        std::reverse(sorted.begin(), sorted.end());
    }
    order_ = std::move(sorted);
    sort_state_ = {key, descending};
}

void ITEM_STORE::restore(const std::vector<uint64_t> &order, const SORT_STATE &sort_state)
{
    for (const uint64_t id : order) {
        if (!items_.contains(id)) {
            throw std::invalid_argument("restored order has an unknown item");
        }
    }
    order_ = order;
    keys_.clear();
    for (const uint64_t id : order_) {
        keys_.emplace(items_.at(id).key, id);
    }
    sort_state_ = sort_state;
}

void ITEM_STORE::prune()
{
    const std::unordered_set<uint64_t> listed(order_.begin(), order_.end());
    std::erase_if(items_, [&listed](const auto &entry) {
        return !listed.contains(entry.first);
    });
}

void ITEM_STORE::relocate(const std::unordered_map<uint64_t, std::filesystem::path> &targets,
    const FOLDER_MOVES &folders)
{
    std::vector<std::pair<uint64_t, std::filesystem::path>> moves;
    for (const auto &[id, item] : items_) {
        const auto target = targets.find(id);
        if (target != targets.end()) {
            moves.emplace_back(id, target->second);
            continue;
        }
        std::optional<std::filesystem::path> moved = folders.follow(item.path);
        if (moved) {
            moves.emplace_back(id, std::move(*moved));
        }
    }

    std::vector<bool> listed;
    listed.reserve(moves.size());
    for (size_t index = 0; index < moves.size(); ++index) {
        listed.push_back(contains(moves[index].first));
    }
    for (size_t index = 0; index < moves.size(); ++index) {
        if (listed[index]) {
            keys_.erase(items_.at(moves[index].first).key);
        }
    }
    for (size_t index = 0; index < moves.size(); ++index) {
        const uint64_t id = moves[index].first;
        const std::filesystem::path &path = moves[index].second;
        ITEM &item = items_.at(id);
        item.path = path;
        item.folder = path.parent_path();
        item.name = text_service_.to_nfc(UTF::path_to_u32(path.filename()).value());
        item.key = key_of(path).value();
        if (listed[index]) {
            keys_.emplace(item.key, id);
        }
    }
}

const std::vector<uint64_t> &ITEM_STORE::order() const
{
    return order_;
}

const SORT_STATE &ITEM_STORE::sort_state() const
{
    return sort_state_;
}

size_t ITEM_STORE::size() const
{
    return order_.size();
}

bool ITEM_STORE::contains(uint64_t id) const
{
    const ITEM *item = find(id);
    if (item == nullptr) {
        return false;
    }
    const auto key = keys_.find(item->key);
    return key != keys_.end() && key->second == id;
}

std::optional<uint64_t> ITEM_STORE::listed_at(const std::filesystem::path &path) const
{
    const std::optional<std::u32string> key = key_of(path);
    if (!key) {
        return std::nullopt;
    }
    const auto found = keys_.find(*key);
    if (found == keys_.end()) {
        return std::nullopt;
    }
    return found->second;
}

const ITEM *ITEM_STORE::find(uint64_t id) const
{
    const auto item = items_.find(id);
    return item == items_.end() ? nullptr : &item->second;
}

std::optional<std::filesystem::path> ITEM_STORE::normalize(const std::filesystem::path &path)
{
    if (!path.is_absolute()) {
        return std::nullopt;
    }
    std::filesystem::path normal = path.lexically_normal();
    if (!normal.has_filename()) {
        normal = normal.parent_path();
    }
    if (!normal.has_filename() || normal == normal.root_path()) {
        return std::nullopt;
    }
    return normal;
}

std::optional<std::u32string> ITEM_STORE::key_of(const std::filesystem::path &path) const
{
    const std::optional<std::u32string> text = UTF::path_to_u32(path);
    if (!text) {
        return std::nullopt;
    }
    return text_service_.fold_key(*text);
}

void ITEM_STORE::add_item(const std::filesystem::path &raw, ADD_RESULT &result)
{
    const std::optional<std::filesystem::path> path = normalize(raw);
    if (!path) {
        ++result.failed;
        return;
    }
    std::optional<std::u32string> key = key_of(*path);
    if (!key) {
        ++result.failed;
        return;
    }
    if (keys_.contains(*key)) {
        ++result.skipped;
        return;
    }
    FILE_INFO info;
    if (!file_system_.get_info(*path, info).ok()) {
        ++result.failed;
        return;
    }
    insert(*path, std::move(*key), info);
    ++result.added;
}

void ITEM_STORE::add_contents(const std::filesystem::path &raw, ADD_RESULT &result)
{
    const std::optional<std::filesystem::path> folder = normalize(raw);
    FILE_INFO folder_info;
    if (!folder || !UTF::path_to_u32(*folder)
        || !file_system_.get_info(*folder, folder_info).ok() || !folder_info.is_dir) {
        ++result.failed;
        return;
    }
    std::vector<std::filesystem::path> names;
    if (!file_system_.list_names(*folder, names).ok()) {
        ++result.failed;
        return;
    }

    std::vector<std::pair<std::u32string, std::filesystem::path>> children;
    for (const std::filesystem::path &name : names) {
        const std::optional<std::u32string> text = UTF::path_to_u32(name);
        if (!text) {
            ++result.failed;
            continue;
        }
        if (!text->empty() && text->front() == U'.') {
            continue;
        }
        children.emplace_back(text_service_.to_nfc(*text), *folder / name);
    }
    std::sort(children.begin(), children.end(), [this](const auto &a, const auto &b) {
        return compare_.compare(a.first, b.first, true) < 0;
    });

    for (const auto &[name, path] : children) {
        std::optional<std::u32string> key = key_of(path);
        if (!key) {
            ++result.failed;
            continue;
        }
        if (keys_.contains(*key)) {
            ++result.skipped;
            continue;
        }
        FILE_INFO info;
        if (!file_system_.get_info(path, info).ok()) {
            ++result.failed;
            continue;
        }
        if (info.is_dir || info.hidden) {
            continue;
        }
        insert(path, std::move(*key), info);
        ++result.added;
    }
}

void ITEM_STORE::insert(const std::filesystem::path &path, std::u32string key,
    const FILE_INFO &info)
{
    ITEM item;
    item.id = next_id_++;
    item.path = path;
    item.folder = path.parent_path();
    item.name = text_service_.to_nfc(UTF::path_to_u32(path.filename()).value());
    item.key = std::move(key);
    item.info = info;
    keys_.emplace(item.key, item.id);
    order_.push_back(item.id);
    items_.emplace(item.id, std::move(item));
}
