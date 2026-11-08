#pragma once

#include <memory>
#include <stdexcept>
#include <utility>

#include "file_system.h"
#include "text_service.h"

class SESSION
{
public:
    static constexpr const char VERSION_TOKEN[] = "LIMENAMER_VERSION=0.1.0";

    SESSION(std::unique_ptr<TEXT_SERVICE> text_service, std::unique_ptr<FILE_SYSTEM> file_system);
    SESSION(const SESSION &) = delete;
    SESSION &operator=(const SESSION &) = delete;

    static const char *version();

    const TEXT_SERVICE &text_service() const;
    FILE_SYSTEM &file_system();

private:
    std::unique_ptr<TEXT_SERVICE> text_service_;
    std::unique_ptr<FILE_SYSTEM> file_system_;
};
