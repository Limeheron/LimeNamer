#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "modification.h"
#include "name_parts.h"
#include "span.h"
#include "span_finder.h"
#include "text_service.h"

class MOD_APPLIER
{
public:
    explicit MOD_APPLIER(const TEXT_SERVICE &text_service);

    std::u32string apply(const MODIFICATION &mod, std::u32string_view name, bool is_dir,
        std::u32string_view value) const;

private:
    static std::u32string edit(std::u32string_view text, const std::vector<SPAN> &spans,
        MOD_KIND kind, std::u32string_view value);

    const TEXT_SERVICE &text_service_;
    SPAN_FINDER finder_;
};
