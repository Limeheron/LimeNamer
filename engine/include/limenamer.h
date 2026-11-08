#pragma once

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

LN_API const char *ln_version(void);

#ifdef __cplusplus
}
#endif
