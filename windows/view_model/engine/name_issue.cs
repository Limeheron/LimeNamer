using System;

namespace limenamer;

[Flags]
public enum NAME_ISSUE : uint
{
    none = 0,
    empty = 1u << 0,
    dot_name = 1u << 1,
    win_forbidden_char = 1u << 2,
    win_reserved = 1u << 3,
    win_trailing = 1u << 4,
    win_name_too_long = 1u << 5,
    win_path_too_long = 1u << 6,
    mac_forbidden_char = 1u << 7,
    mac_name_too_long = 1u << 8,
    hidden = 1u << 9
}
