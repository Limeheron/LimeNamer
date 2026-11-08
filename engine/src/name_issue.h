#pragma once

#include <cstdint>

class NAME_ISSUE
{
public:
    static constexpr const uint32_t EMPTY = 1u << 0;
    static constexpr const uint32_t DOT_NAME = 1u << 1;
    static constexpr const uint32_t WIN_FORBIDDEN_CHAR = 1u << 2;
    static constexpr const uint32_t WIN_RESERVED = 1u << 3;
    static constexpr const uint32_t WIN_TRAILING = 1u << 4;
    static constexpr const uint32_t WIN_NAME_TOO_LONG = 1u << 5;
    static constexpr const uint32_t WIN_PATH_TOO_LONG = 1u << 6;
    static constexpr const uint32_t MAC_FORBIDDEN_CHAR = 1u << 7;
    static constexpr const uint32_t MAC_NAME_TOO_LONG = 1u << 8;
    static constexpr const uint32_t HIDDEN = 1u << 9;

    static constexpr const uint32_t COMMON = EMPTY | DOT_NAME;
    static constexpr const uint32_t WINDOWS = WIN_FORBIDDEN_CHAR | WIN_RESERVED | WIN_TRAILING
        | WIN_NAME_TOO_LONG | WIN_PATH_TOO_LONG;
    static constexpr const uint32_t MAC = MAC_FORBIDDEN_CHAR | MAC_NAME_TOO_LONG;
};
