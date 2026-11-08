#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "paths.h"

class FOLDER_MOVES
{
public:
    void add(const std::filesystem::path &from, const std::filesystem::path &to);
    bool empty() const;
    std::optional<std::filesystem::path> follow(const std::filesystem::path &path) const;

private:
    std::unordered_map<std::filesystem::path::string_type, std::filesystem::path> moves_;
};
