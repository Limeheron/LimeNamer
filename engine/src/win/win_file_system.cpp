#include "win_file_system.h"

FS_STATUS WIN_FILE_SYSTEM::get_info(const std::filesystem::path &path, FILE_INFO &info) const
{
    const std::wstring target = extended(path);
    const HANDLE raw = CreateFileW(target.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (raw == INVALID_HANDLE_VALUE) {
        return from_error(GetLastError());
    }
    const SCOPED_HANDLE handle(raw, &CloseHandle);
    BY_HANDLE_FILE_INFORMATION basic {};
    if (!GetFileInformationByHandle(handle.get(), &basic)) {
        return from_error(GetLastError());
    }
    info.is_dir = (basic.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    const uint64_t size = (static_cast<uint64_t>(basic.nFileSizeHigh) << 32) | basic.nFileSizeLow;
    info.size = info.is_dir ? 0 : static_cast<int64_t>(size);
    info.modified_ms = to_ms(basic.ftLastWriteTime);
    info.created_ms = to_ms(basic.ftCreationTime);

    FILE_ID_INFO id {};
    if (GetFileInformationByHandleEx(handle.get(), FileIdInfo, &id, sizeof(id))) {
        info.id.volume = id.VolumeSerialNumber;
        std::memcpy(&info.id.low, id.FileId.Identifier, sizeof(uint64_t));
        std::memcpy(&info.id.high, id.FileId.Identifier + sizeof(uint64_t), sizeof(uint64_t));
    } else {
        info.id.volume = basic.dwVolumeSerialNumber;
        info.id.high = 0;
        info.id.low = (static_cast<uint64_t>(basic.nFileIndexHigh) << 32) | basic.nFileIndexLow;
    }
    info.id.valid = info.id.high != 0 || info.id.low != 0;
    return {};
}

FS_STATUS WIN_FILE_SYSTEM::list_names(const std::filesystem::path &folder,
    std::vector<std::filesystem::path> &names) const
{
    const std::wstring base = extended(folder);
    std::wstring pattern = base;
    if (!pattern.ends_with(L'\\')) {
        pattern += L'\\';
    }
    pattern += L'*';
    WIN32_FIND_DATAW data {};
    const HANDLE raw = FindFirstFileExW(pattern.c_str(), FindExInfoBasic, &data,
        FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
    if (raw == INVALID_HANDLE_VALUE) {
        const DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND && is_directory(base)) {
            names.clear();
            return {};
        }
        return from_error(error == ERROR_FILE_NOT_FOUND ? ERROR_PATH_NOT_FOUND : error);
    }
    const SCOPED_FIND find(raw, &FindClose);
    std::vector<std::filesystem::path> result;
    do {
        const std::wstring_view name = data.cFileName;
        if (name != L"." && name != L"..") {
            result.emplace_back(std::wstring(name));
        }
    } while (FindNextFileW(find.get(), &data));
    const DWORD error = GetLastError();
    if (error != ERROR_NO_MORE_FILES) {
        return from_error(error);
    }
    names = std::move(result);
    return {};
}

FS_STATUS WIN_FILE_SYSTEM::rename(const std::filesystem::path &from,
    const std::filesystem::path &to)
{
    if (!MoveFileExW(extended(from).c_str(), extended(to).c_str(), 0)) {
        return from_error(GetLastError());
    }
    return {};
}

std::wstring WIN_FILE_SYSTEM::extended(const std::filesystem::path &path)
{
    const std::wstring &raw = path.native();
    if (raw.starts_with(L"\\\\?\\") || !path.is_absolute()) {
        return raw;
    }
    std::filesystem::path normal = path.lexically_normal();
    normal.make_preferred();
    std::wstring text = normal.native();
    while (text.size() > 3 && text.back() == L'\\') {
        text.pop_back();
    }
    if (text.starts_with(L"\\\\")) {
        return L"\\\\?\\UNC\\" + text.substr(2);
    }
    return L"\\\\?\\" + text;
}

bool WIN_FILE_SYSTEM::is_directory(const std::wstring &path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

FS_STATUS WIN_FILE_SYSTEM::from_error(DWORD error)
{
    FS_STATUS status;
    status.os_error = static_cast<int32_t>(error);
    switch (error) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_DIRECTORY:
    case ERROR_INVALID_DRIVE:
    case ERROR_BAD_NETPATH:
    case ERROR_BAD_NET_NAME:
        status.error = FS_ERROR::not_found;
        break;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
        status.error = FS_ERROR::already_exists;
        break;
    case ERROR_ACCESS_DENIED:
    case ERROR_WRITE_PROTECT:
        status.error = FS_ERROR::access_denied;
        break;
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
        status.error = FS_ERROR::in_use;
        break;
    case ERROR_FILENAME_EXCED_RANGE:
        status.error = FS_ERROR::path_too_long;
        break;
    default:
        status.error = FS_ERROR::other;
        break;
    }
    return status;
}

int64_t WIN_FILE_SYSTEM::to_ms(const FILETIME &time)
{
    ULARGE_INTEGER ticks {};
    ticks.LowPart = time.dwLowDateTime;
    ticks.HighPart = time.dwHighDateTime;
    return (static_cast<int64_t>(ticks.QuadPart) - EPOCH_TICKS) / 10000;
}
