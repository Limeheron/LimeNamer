#pragma once

enum class RENAME_FAILURE
{
    none,
    in_use,
    access_denied,
    not_found,
    already_exists,
    replaced,
    path_too_long,
    depends_on_failed,
    other
};
