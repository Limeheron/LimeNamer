#pragma once

#include <stdexcept>

class SESSION_BUSY : public std::runtime_error
{
public:
    SESSION_BUSY()
        : std::runtime_error("session is waiting for a rename choice")
    {
    }
};
