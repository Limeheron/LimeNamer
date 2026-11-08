#include "paths.h"

bool PATHS::is_under(const std::filesystem::path &path, const std::filesystem::path &base)
{
    auto part = path.begin();
    for (const std::filesystem::path &component : base) {
        if (part == path.end() || *part != component) {
            return false;
        }
        ++part;
    }
    return part != path.end();
}

std::filesystem::path PATHS::moved(const std::filesystem::path &path,
    const std::filesystem::path &from, const std::filesystem::path &to)
{
    return to / path.lexically_relative(from);
}
