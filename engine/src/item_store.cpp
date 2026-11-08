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
}

const std::vector<uint64_t> &ITEM_STORE::order() const
{
    return order_;
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
