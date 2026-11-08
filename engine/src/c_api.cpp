#include "c_api.h"

namespace
{
template <typename BODY>
LN_RESULT guard(BODY &&body) noexcept
{
    try {
        return body();
    } catch (const std::bad_alloc &) {
        return ln_result_out_of_memory;
    } catch (...) {
        return ln_result_internal;
    }
}

SESSION *session_of(LN_SESSION *session)
{
    return reinterpret_cast<SESSION *>(session);
}
}

LN_RESULT ln_session_create(LN_SESSION **out)
{
    if (out == nullptr) {
        return ln_result_invalid_argument;
    }
    *out = nullptr;
    return guard([out]() -> LN_RESULT {
        std::unique_ptr<SESSION> session = std::make_unique<SESSION>(
            PLATFORM::create_text_service(), PLATFORM::create_file_system());
        *out = reinterpret_cast<LN_SESSION *>(session.release());
        return ln_result_ok;
    });
}

void ln_session_destroy(LN_SESSION *session)
{
    delete session_of(session);
}

const char *ln_version(void)
{
    return SESSION::version();
}
