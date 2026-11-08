#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "add_mode.h"
#include "add_result.h"
#include "file_system.h"
#include "item.h"
#include "natural_compare.h"
#include "paths.h"
#include "sort_key.h"
#include "sort_state.h"
#include "text_service.h"
#include "utf.h"

class ITEM_STORE
{
public:
    ITEM_STORE(const FILE_SYSTEM &file_system, const TEXT_SERVICE &text_service);

    ADD_RESULT add(const std::vector<std::filesystem::path> &paths, ADD_MODE mode);
    size_t remove(const std::vector<uint64_t> &ids);
    void clear();
    std::optional<bool> move(const std::vector<uint64_t> &ids, uint64_t target_id, bool after,
        bool dry_run);
    void sort(SORT_KEY key, bool descending);
    void restore(const std::vector<uint64_t> &order, const SORT_STATE &sort_state);
    void prune();
    void relocate(uint64_t id, const std::filesystem::path &path);
    void move_folder(const std::filesystem::path &from, const std::filesystem::path &to);

    const std::vector<uint64_t> &order() const;
    const SORT_STATE &sort_state() const;
    size_t size() const;
    bool contains(uint64_t id) const;
    const ITEM *find(uint64_t id) const;

private:
    static std::optional<std::filesystem::path> normalize(const std::filesystem::path &path);

    std::optional<std::u32string> key_of(const std::filesystem::path &path) const;
    void add_item(const std::filesystem::path &raw, ADD_RESULT &result);
    void add_contents(const std::filesystem::path &raw, ADD_RESULT &result);
    void insert(const std::filesystem::path &path, std::u32string key, const FILE_INFO &info);

    const FILE_SYSTEM &file_system_;
    const TEXT_SERVICE &text_service_;
    NATURAL_COMPARE compare_;
    std::unordered_map<uint64_t, ITEM> items_;
    std::vector<uint64_t> order_;
    std::unordered_map<std::u32string, uint64_t> keys_;
    SORT_STATE sort_state_;
    uint64_t next_id_ = 1;
};
