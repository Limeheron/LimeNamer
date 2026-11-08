#pragma once

#include <atomic>
#include <exception>
#include <new>
#include <stdexcept>

#include "limenamer.h"
#include "ln_session.h"
#include "mod_json_error.h"
#include "session.h"
#include "session_busy.h"

class C_GUARD
{
public:
    template <typename BODY>
    static LN_RESULT run(BODY &&body) noexcept
    {
        try {
            return body();
        } catch (...) {
            return current_error();
        }
    }

    template <typename BODY>
    static LN_RESULT run(LN_SESSION *session, BODY &&body) noexcept
    {
        if (session == nullptr) {
            return ln_result_invalid_argument;
        }
        bool expected = false;
        if (!session->busy.compare_exchange_strong(expected, true)) {
            return ln_result_busy;
        }
        const LN_RESULT result = run([&]() -> LN_RESULT { return body(session->session); });
        session->busy.store(false);
        return result;
    }

private:
    static LN_RESULT current_error() noexcept;
};
