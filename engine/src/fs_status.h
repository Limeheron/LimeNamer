#pragma once

#include <cstdint>

#include "fs_error.h"

struct FS_STATUS
{
    FS_ERROR error = FS_ERROR::none;
    int32_t os_error = 0;

    bool ok() const
    {
        return error == FS_ERROR::none;
    }
};
