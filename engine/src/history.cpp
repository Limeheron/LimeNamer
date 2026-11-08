#include "history.h"

void HISTORY::record(SNAPSHOT before, const SNAPSHOT &after)
{
    if (before == after) {
        return;
    }
    push(undo_, std::move(before));
    redo_.clear();
}

std::optional<SNAPSHOT> HISTORY::undo(SNAPSHOT current)
{
    if (undo_.empty()) {
        return std::nullopt;
    }
    SNAPSHOT previous = std::move(undo_.back());
    undo_.pop_back();
    push(redo_, std::move(current));
    return previous;
}

std::optional<SNAPSHOT> HISTORY::redo(SNAPSHOT current)
{
    if (redo_.empty()) {
        return std::nullopt;
    }
    SNAPSHOT next = std::move(redo_.back());
    redo_.pop_back();
    push(undo_, std::move(current));
    return next;
}

void HISTORY::clear()
{
    undo_.clear();
    redo_.clear();
}

bool HISTORY::can_undo() const
{
    return !undo_.empty();
}

bool HISTORY::can_redo() const
{
    return !redo_.empty();
}

void HISTORY::push(std::deque<SNAPSHOT> &stack, SNAPSHOT snapshot)
{
    stack.push_back(std::move(snapshot));
    if (stack.size() > MAX_STEPS) {
        stack.pop_front();
    }
}
