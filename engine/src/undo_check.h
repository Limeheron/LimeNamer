#pragma once

#include <vector>

#include "rename_request.h"
#include "report_item.h"

struct UNDO_CHECK
{
    std::vector<RENAME_REQUEST> requests;
    std::vector<REPORT_ITEM> skipped;
};
