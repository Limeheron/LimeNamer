#include "c_api.h"

const char *ln_version(void)
{
    return SESSION::version();
}
