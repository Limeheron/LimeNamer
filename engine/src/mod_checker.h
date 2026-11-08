#pragma once

#include <string>
#include <variant>
#include <vector>

#include "mod_problem.h"
#include "modification.h"

class MOD_CHECKER
{
public:
    static std::vector<MOD_PROBLEM> check(const MODIFICATION &mod);

private:
    static bool has_value(const std::vector<VALUE_PIECE> &value);
    static bool uses(const std::vector<VALUE_PIECE> &value, TOKEN_KIND token);
};
