#include "c_guard.h"

LN_RESULT C_GUARD::current_error() noexcept
{
    try {
        throw;
    } catch (const MOD_JSON_ERROR &) {
        return ln_result_invalid_json;
    } catch (const SESSION_BUSY &) {
        return ln_result_busy;
    } catch (const std::bad_alloc &) {
        return ln_result_out_of_memory;
    } catch (const std::invalid_argument &) {
        return ln_result_invalid_argument;
    } catch (...) {
        return ln_result_internal;
    }
}
