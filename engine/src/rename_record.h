#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "file_info.h"
#include "file_system.h"
#include "record_entry.h"
#include "rename_executor.h"
#include "rename_failure.h"
#include "rename_request.h"
#include "report_item.h"
#include "text_service.h"
#include "undo_check.h"
#include "utf.h"

class RENAME_RECORD
{
public:
    void set(std::vector<RECORD_ENTRY> entries);
    void clear();

    bool empty() const;
    size_t size() const;
    const std::vector<RECORD_ENTRY> &entries() const;

    UNDO_CHECK check(const FILE_SYSTEM &file_system, const TEXT_SERVICE &text_service) const;

    static bool same_file(const FILE_INFO &recorded, const FILE_INFO &now);

private:
    std::vector<RECORD_ENTRY> entries_;
};
