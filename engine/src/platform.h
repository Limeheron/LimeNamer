#pragma once

#include <memory>

#include "text_service.h"

#if defined(_WIN32)
#include "win/win_text_service.h"
#elif defined(__APPLE__)
#include "mac/mac_text_service.h"
#else
#error "unsupported platform"
#endif

class PLATFORM
{
public:
    static std::unique_ptr<TEXT_SERVICE> create_text_service();
};
