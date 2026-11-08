#pragma once

#include <cstdint>
#include <vector>

#include "index_error.h"

struct INDEX_LIST_RESULT
{
    std::vector<uint32_t> indices;
    INDEX_ERROR error = INDEX_ERROR::none;
};
