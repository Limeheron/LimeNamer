#pragma once

#include <string>
#include <variant>

#include "token_type.h"

using VALUE_PIECE = std::variant<std::u32string, TOKEN_TYPE>;
