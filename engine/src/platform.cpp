#include "platform.h"

std::unique_ptr<TEXT_SERVICE> PLATFORM::create_text_service()
{
#if defined(_WIN32)
    return std::make_unique<WIN_TEXT_SERVICE>();
#else
    return std::make_unique<MAC_TEXT_SERVICE>();
#endif
}
