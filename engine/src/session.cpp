#include "session.h"

SESSION::SESSION(std::unique_ptr<TEXT_SERVICE> text_service,
    std::unique_ptr<FILE_SYSTEM> file_system)
    : text_service_(std::move(text_service)),
      file_system_(std::move(file_system))
{
    if (text_service_ == nullptr || file_system_ == nullptr) {
        throw std::invalid_argument("session requires text service and file system");
    }
}

const char *SESSION::version()
{
    constexpr const char PREFIX[] = "LIMENAMER_VERSION=";
    return VERSION_TOKEN + sizeof(PREFIX) - 1;
}

const TEXT_SERVICE &SESSION::text_service() const
{
    return *text_service_;
}

FILE_SYSTEM &SESSION::file_system()
{
    return *file_system_;
}
