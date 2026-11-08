#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include "fake_file_system.h"
#include "file_system.h"
#include "platform.h"
#include "text_service.h"

class FAKE_FS_ENV
{
public:
    FAKE_FS_ENV();

    FILE_SYSTEM &file_system();
    FAKE_FILE_SYSTEM &fake();
    const TEXT_SERVICE &text_service() const;
    const std::filesystem::path &root() const;
    void make_file(const std::filesystem::path &path, int64_t size);
    void make_folder(const std::filesystem::path &path);

private:
    std::unique_ptr<TEXT_SERVICE> text_service_;
    FAKE_FILE_SYSTEM file_system_;
    std::filesystem::path root_;
};
