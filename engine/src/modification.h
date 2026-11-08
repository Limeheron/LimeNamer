#pragma once

#include <cstdint>
#include <vector>

#include "format_settings.h"
#include "mod_kind.h"
#include "mod_part.h"
#include "mod_scope.h"
#include "number_settings.h"
#include "position.h"
#include "value_piece.h"

struct MODIFICATION
{
    MOD_KIND kind = MOD_KIND::insert;
    MOD_PART part = MOD_PART::name;
    POSITION position;
    std::vector<VALUE_PIECE> value;
    NUMBER_SETTINGS number;
    FORMAT_SETTINGS format;
    MOD_SCOPE scope = MOD_SCOPE::all;
    std::vector<uint64_t> targets;
    bool direct = false;

    bool operator==(const MODIFICATION &other) const = default;
};
