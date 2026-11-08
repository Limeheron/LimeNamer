#pragma once

#include <stdint.h>

#if defined(_WIN32)
#if defined(LN_BUILDING)
#define LN_API __declspec(dllexport)
#else
#define LN_API __declspec(dllimport)
#endif
#else
#define LN_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef int32_t LN_RESULT;

enum
{
    ln_result_ok = 0,
    ln_result_invalid_argument = 1,
    ln_result_invalid_json = 2,
    ln_result_not_found = 3,
    ln_result_busy = 4,
    ln_result_not_ready = 5,
    ln_result_io = 6,
    ln_result_out_of_memory = 7,
    ln_result_internal = 8
};

typedef struct LN_SESSION LN_SESSION;

LN_API LN_RESULT ln_session_create(LN_SESSION **out);
LN_API void ln_session_destroy(LN_SESSION *session);
LN_API const char *ln_version(void);

#ifdef __cplusplus
}
#endif
