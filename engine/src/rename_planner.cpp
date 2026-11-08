#include "rename_planner.h"

RENAME_PLANNER::RENAME_PLANNER(const TEXT_SERVICE &text_service)
    : text_service_(text_service)
{
}

std::vector<RENAME_STEP> RENAME_PLANNER::plan(const std::vector<RENAME_REQUEST> &requests,
    const FOLDER_CACHE &cache) const
{
    std::vector<std::u32string> folder_order;
    std::unordered_map<std::u32string, std::vector<const RENAME_REQUEST *>> groups;
    std::unordered_map<std::u32string, size_t> depths;
    for (const RENAME_REQUEST &request : requests) {
        const std::filesystem::path folder = request.path.parent_path().lexically_normal();
        const std::u32string key = cache.key_of(folder).value_or(std::u32string());
        if (!groups.contains(key)) {
            folder_order.push_back(key);
            depths.emplace(key, static_cast<size_t>(std::distance(folder.begin(), folder.end())));
        }
        groups[key].push_back(&request);
    }
    std::stable_sort(folder_order.begin(), folder_order.end(),
        [&depths](const std::u32string &a, const std::u32string &b) {
            return depths.at(a) > depths.at(b);
        });

    std::vector<RENAME_STEP> steps;
    steps.reserve(requests.size());
    for (const std::u32string &key : folder_order) {
        plan_folder(groups.at(key), cache.find_key(key), steps);
    }
    return steps;
}

void RENAME_PLANNER::plan_folder(const std::vector<const RENAME_REQUEST *> &requests,
    const FOLDER_NAMES *existing, std::vector<RENAME_STEP> &steps) const
{
    const size_t count = requests.size();
    std::vector<std::u32string> current_keys(count);
    std::vector<std::u32string> target_keys(count);
    std::vector<std::filesystem::path> current_paths(count);
    std::vector<bool> in_temp(count, false);
    std::vector<bool> done(count, false);
    std::unordered_map<std::u32string, size_t> occupied;
    std::unordered_set<std::u32string> reserved;
    for (size_t index = 0; index < count; ++index) {
        current_keys[index] = text_service_.fold_key(requests[index]->name);
        target_keys[index] = text_service_.fold_key(requests[index]->new_name);
        current_paths[index] = requests[index]->path;
        occupied.emplace(current_keys[index], index);
        reserved.insert(current_keys[index]);
        reserved.insert(target_keys[index]);
    }

    std::unordered_map<std::u32string, std::vector<size_t>> waiting;
    std::deque<size_t> ready;
    for (size_t index = 0; index < count; ++index) {
        if (occupied.contains(target_keys[index])) {
            waiting[target_keys[index]].push_back(index);
        } else {
            ready.push_back(index);
        }
    }
    const auto release = [&](size_t index) {
        const auto holder = occupied.find(current_keys[index]);
        if (holder == occupied.end() || holder->second != index) {
            return;
        }
        occupied.erase(holder);
        const auto blocked = waiting.find(current_keys[index]);
        if (blocked != waiting.end()) {
            ready.insert(ready.end(), blocked->second.begin(), blocked->second.end());
            waiting.erase(blocked);
        }
    };

    uint64_t temp_number = 1;
    size_t next_pending = 0;
    while (true) {
        while (!ready.empty()) {
            const size_t index = ready.front();
            ready.pop_front();
            if (done[index]) {
                continue;
            }
            const RENAME_REQUEST &request = *requests[index];
            steps.push_back({request.item_id, current_paths[index],
                request.path.parent_path() / UTF::path_from_u32(request.new_name),
                in_temp[index] ? STEP_KIND::from_temp : STEP_KIND::direct});
            done[index] = true;
            release(index);
        }
        while (next_pending < count && done[next_pending]) {
            ++next_pending;
        }
        if (next_pending == count) {
            break;
        }

        const size_t index = next_pending;
        std::u32string temp;
        std::u32string temp_key;
        while (true) {
            const std::string number = std::to_string(temp_number++);
            temp = TEMP_PREFIX;
            for (const char c : number) {
                temp.push_back(static_cast<char32_t>(c));
            }
            temp_key = text_service_.fold_key(temp);
            const bool exists = existing != nullptr && existing->keys.contains(temp_key);
            if (!exists && !reserved.contains(temp_key)) {
                break;
            }
        }
        reserved.insert(temp_key);
        const std::filesystem::path temp_path =
            requests[index]->path.parent_path() / UTF::path_from_u32(temp);
        steps.push_back({requests[index]->item_id, current_paths[index], temp_path,
            STEP_KIND::to_temp});
        release(index);
        current_paths[index] = temp_path;
        current_keys[index] = temp_key;
        in_temp[index] = true;
    }
}
