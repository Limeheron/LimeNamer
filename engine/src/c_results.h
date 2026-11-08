#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "add_result.h"
#include "c_block.h"
#include "conflict_kind.h"
#include "example.h"
#include "example_state.h"
#include "index_error.h"
#include "index_list_result.h"
#include "limenamer.h"
#include "mod_field.h"
#include "mod_problem.h"
#include "name_issue.h"
#include "preview.h"
#include "problem_reason.h"
#include "rename_failure.h"
#include "rename_report.h"
#include "report_stage.h"
#include "row_status.h"
#include "utf.h"

class C_RESULTS
{
public:
    static LN_PREVIEW *preview(const PREVIEW &preview);
    static LN_EXAMPLE *example(const EXAMPLE &example);
    static LN_REPORT *report(const RENAME_REPORT &report);
    static LN_PROBLEMS *problems(const std::vector<MOD_PROBLEM> &problems);
    static LN_INDEX_LIST *index_list(const INDEX_LIST_RESULT &result);
    static char *string(std::string_view text);
    static LN_ADD_RESULT add_result(const ADD_RESULT &result);
    static void release(void *result);

private:
    static std::string utf8_of(const std::filesystem::path &path);
};
