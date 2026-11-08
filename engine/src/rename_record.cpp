#include "rename_record.h"

void RENAME_RECORD::set(std::vector<RECORD_ENTRY> entries)
{
    entries_ = std::move(entries);
}

void RENAME_RECORD::clear()
{
    entries_.clear();
}

bool RENAME_RECORD::empty() const
{
    return entries_.empty();
}

size_t RENAME_RECORD::size() const
{
    return entries_.size();
}

const std::vector<RECORD_ENTRY> &RENAME_RECORD::entries() const
{
    return entries_;
}
