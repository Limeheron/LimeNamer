#include "session.h"

const char *SESSION::version()
{
    constexpr const char PREFIX[] = "LIMENAMER_VERSION=";
    return VERSION_TOKEN + sizeof(PREFIX) - 1;
}
