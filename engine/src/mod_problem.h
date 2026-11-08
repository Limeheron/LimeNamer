#pragma once

#include "mod_field.h"
#include "problem_reason.h"

struct MOD_PROBLEM
{
    MOD_FIELD field = MOD_FIELD::value;
    PROBLEM_REASON reason = PROBLEM_REASON::required;

    bool operator==(const MOD_PROBLEM &other) const = default;
};
