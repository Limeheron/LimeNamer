#pragma once

#include <ostream>

#include "fs_error.h"

inline std::ostream &operator<<(std::ostream &out, FS_ERROR error)
{
    switch (error) {
    case FS_ERROR::none:
        return out << "none";
    case FS_ERROR::not_found:
        return out << "not_found";
    case FS_ERROR::already_exists:
        return out << "already_exists";
    case FS_ERROR::access_denied:
        return out << "access_denied";
    case FS_ERROR::in_use:
        return out << "in_use";
    case FS_ERROR::path_too_long:
        return out << "path_too_long";
    case FS_ERROR::other:
        return out << "other";
    }
    return out << "unknown";
}
