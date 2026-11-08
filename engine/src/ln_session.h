#pragma once

#include <atomic>
#include <memory>
#include <utility>

#include "file_system.h"
#include "session.h"
#include "text_service.h"

struct LN_SESSION
{
    LN_SESSION(std::unique_ptr<TEXT_SERVICE> text_service,
        std::unique_ptr<FILE_SYSTEM> file_system, bool windows_host)
        : session(std::move(text_service), std::move(file_system), windows_host)
    {
    }

    SESSION session;
    std::atomic<bool> busy = false;
};
