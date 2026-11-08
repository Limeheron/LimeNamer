#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

#include "mod_json_error.h"
#include "modification.h"
#include "utf.h"

class MOD_JSON
{
public:
    static MODIFICATION parse(std::string_view text);
    static std::string to_json(const MODIFICATION &mod);
};
