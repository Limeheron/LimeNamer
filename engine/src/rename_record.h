#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "record_entry.h"

class RENAME_RECORD
{
public:
    void set(std::vector<RECORD_ENTRY> entries);
    void clear();

    bool empty() const;
    size_t size() const;
    const std::vector<RECORD_ENTRY> &entries() const;

private:
    std::vector<RECORD_ENTRY> entries_;
};
