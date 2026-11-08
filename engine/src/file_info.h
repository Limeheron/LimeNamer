#pragma once

#include <cstdint>

#include "file_id.h"

struct FILE_INFO
{
    bool is_dir = false;
    bool hidden = false;
    int64_t size = 0;
    int64_t modified_ms = 0;
    int64_t created_ms = 0;
    FILE_ID id;
};
