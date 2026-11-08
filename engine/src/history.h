#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <utility>

#include "snapshot.h"

class HISTORY
{
public:
    static constexpr const size_t MAX_STEPS = 30;

    void record(SNAPSHOT before, const SNAPSHOT &after);
    std::optional<SNAPSHOT> undo(SNAPSHOT current);
    std::optional<SNAPSHOT> redo(SNAPSHOT current);
    void clear();

    bool can_undo() const;
    bool can_redo() const;

private:
    static void push(std::deque<SNAPSHOT> &stack, SNAPSHOT snapshot);

    std::deque<SNAPSHOT> undo_;
    std::deque<SNAPSHOT> redo_;
};
