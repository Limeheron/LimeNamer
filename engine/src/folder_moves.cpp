#include "folder_moves.h"

void FOLDER_MOVES::add(const std::filesystem::path &from, const std::filesystem::path &to)
{
    moves_.insert_or_assign(from.native(), to);
}

bool FOLDER_MOVES::empty() const
{
    return moves_.empty();
}

std::optional<std::filesystem::path> FOLDER_MOVES::follow(const std::filesystem::path &path) const
{
    if (moves_.empty()) {
        return std::nullopt;
    }
    std::filesystem::path folder = path.parent_path();
    while (folder.has_relative_path()) {
        const auto move = moves_.find(folder.native());
        if (move != moves_.end()) {
            return PATHS::moved(path, folder, move->second);
        }
        folder = folder.parent_path();
    }
    return std::nullopt;
}
