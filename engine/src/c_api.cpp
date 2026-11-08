#include "c_api.h"

LN_RESULT ln_session_create(LN_SESSION **out)
{
    if (out == nullptr) {
        return ln_result_invalid_argument;
    }
    *out = nullptr;
    return C_GUARD::run([out]() -> LN_RESULT {
        *out = std::make_unique<LN_SESSION>(PLATFORM::create_text_service(),
            PLATFORM::create_file_system(), PLATFORM::WINDOWS_HOST).release();
        return ln_result_ok;
    });
}

void ln_session_destroy(LN_SESSION *session)
{
    delete session;
}

const char *ln_version(void)
{
    return SESSION::version();
}

void ln_string_free(char *text)
{
    C_RESULTS::release(text);
}

void ln_problems_free(LN_PROBLEMS *problems)
{
    C_RESULTS::release(problems);
}

void ln_example_free(LN_EXAMPLE *example)
{
    C_RESULTS::release(example);
}

void ln_preview_free(LN_PREVIEW *preview)
{
    C_RESULTS::release(preview);
}

void ln_report_free(LN_REPORT *report)
{
    C_RESULTS::release(report);
}

void ln_index_list_free(LN_INDEX_LIST *list)
{
    C_RESULTS::release(list);
}
