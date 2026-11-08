#pragma once

#include <stdexcept>
#include <string>

class MOD_JSON_ERROR : public std::runtime_error
{
public:
    explicit MOD_JSON_ERROR(const std::string &message)
        : std::runtime_error(message)
    {
    }
};
