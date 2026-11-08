#pragma once

#include <filesystem>

class PATHS
{
public:
    static bool is_under(const std::filesystem::path &path, const std::filesystem::path &base);
    static std::filesystem::path moved(const std::filesystem::path &path,
        const std::filesystem::path &from, const std::filesystem::path &to);
};
