#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "mod_entry.h"
#include "modification.h"
#include "text_service.h"

class MOD_STORE
{
public:
    explicit MOD_STORE(const TEXT_SERVICE &text_service);

    uint64_t add(MODIFICATION mod);
    bool update(uint64_t id, MODIFICATION mod);
    bool remove(uint64_t id);
    std::optional<bool> move(uint64_t id, uint64_t target_id, bool after, bool dry_run);
    bool set_enabled(uint64_t id, bool enabled);
    void set_all_enabled(bool enabled);
    uint64_t direct_edit(uint64_t item_id, std::u32string new_name);

    const std::vector<MOD_ENTRY> &entries() const;
    const MOD_ENTRY *find(uint64_t id) const;

private:
    std::optional<size_t> index_of(uint64_t id) const;
    void normalize(MODIFICATION &mod) const;

    const TEXT_SERVICE &text_service_;
    std::vector<MOD_ENTRY> entries_;
    uint64_t next_id_ = 1;
};
