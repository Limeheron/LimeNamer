#pragma once

#include <string>
#include <variant>

#include "token_kind.h"

using VALUE_PIECE = std::variant<std::u32string, TOKEN_KIND>;
