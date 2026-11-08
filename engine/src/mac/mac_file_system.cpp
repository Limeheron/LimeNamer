#include "mac_file_system.h"

FS_STATUS MAC_FILE_SYSTEM::get_info(const std::filesystem::path &path, FILE_INFO &info) const
{
    struct stat status {};
    if (lstat(path.c_str(), &status) != 0) {
        return from_errno(errno);
    }
    info.is_dir = S_ISDIR(status.st_mode);
    info.size = info.is_dir ? 0 : static_cast<int64_t>(status.st_size);
    info.modified_ms = to_ms(status.st_mtimespec);
    info.created_ms = to_ms(status.st_birthtimespec);
    info.id.valid = true;
    info.id.volume = static_cast<uint32_t>(status.st_dev);
    info.id.high = 0;
    info.id.low = status.st_ino;
    return {};
}

FS_STATUS MAC_FILE_SYSTEM::list_names(const std::filesystem::path &folder,
    std::vector<std::filesystem::path> &names) const
{
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(folder.c_str()), &closedir);
    if (dir == nullptr) {
        return from_errno(errno);
    }
    std::vector<std::filesystem::path> result;
    while (true) {
        errno = 0;
        const dirent *entry = readdir(dir.get());
        if (entry == nullptr) {
            break;
        }
        const std::string_view name = entry->d_name;
        if (name != "." && name != "..") {
            result.emplace_back(name);
        }
    }
    if (errno != 0) {
        return from_errno(errno);
    }
    names = std::move(result);
    return {};
}

FS_STATUS MAC_FILE_SYSTEM::rename(const std::filesystem::path &from,
    const std::filesystem::path &to)
{
    if (renamex_np(from.c_str(), to.c_str(), RENAME_EXCL) != 0) {
        return from_errno(errno);
    }
    return {};
}

FS_STATUS MAC_FILE_SYSTEM::from_errno(int error)
{
    FS_STATUS status;
    status.os_error = error;
    switch (error) {
    case ENOENT:
    case ENOTDIR:
        status.error = FS_ERROR::not_found;
        break;
    case EEXIST:
    case ENOTEMPTY:
        status.error = FS_ERROR::already_exists;
        break;
    case EACCES:
    case EPERM:
    case EROFS:
        status.error = FS_ERROR::access_denied;
        break;
    case EBUSY:
        status.error = FS_ERROR::in_use;
        break;
    case ENAMETOOLONG:
        status.error = FS_ERROR::path_too_long;
        break;
    default:
        status.error = FS_ERROR::other;
        break;
    }
    return status;
}

int64_t MAC_FILE_SYSTEM::to_ms(const timespec &time)
{
    return static_cast<int64_t>(time.tv_sec) * 1000 + time.tv_nsec / 1000000;
}
