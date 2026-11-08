#pragma once

enum class FS_ERROR
{
    none,
    not_found,
    already_exists,
    access_denied,
    in_use,
    path_too_long,
    other
};
