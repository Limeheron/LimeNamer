#pragma once

#include <memory>

#include "file_system.h"
#include "text_service.h"

#if defined(_WIN32)
#include "win/win_file_system.h"
#include "win/win_text_service.h"
#elif defined(__APPLE__)
#include "mac/mac_file_system.h"
#include "mac/mac_text_service.h"
#else
#error "unsupported platform"
#endif

class PLATFORM
{
public:
    static std::unique_ptr<TEXT_SERVICE> create_text_service();
    static std::unique_ptr<FILE_SYSTEM> create_file_system();
};
