#include "file_system.h"

FS_STATUS FILE_SYSTEM::list_entries(const std::filesystem::path &folder,
    std::vector<FOLDER_ENTRY> &entries) const
{
    std::vector<std::filesystem::path> names;
    const FS_STATUS status = list_names(folder, names);
    if (!status.ok()) {
        return status;
    }
    std::vector<FOLDER_ENTRY> result(names.size());
    for (size_t index = 0; index < names.size(); ++index) {
        result[index].status = get_info(folder / names[index], result[index].info);
        result[index].name = std::move(names[index]);
    }
    entries = std::move(result);
    return {};
}
