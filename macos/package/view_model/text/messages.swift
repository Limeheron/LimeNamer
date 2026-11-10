public enum MESSAGES
{
    public static func status_name(_ status: STATUS) -> String
    {
        switch status {
        case .ok:
            return STRINGS.status_ok
        case .unchanged:
            return STRINGS.status_unchanged
        case .warning:
            return STRINGS.status_warning
        case .error:
            return STRINGS.status_error
        case .conflict:
            return STRINGS.status_conflict
        }
    }

    public static func row_messages(_ row: ROW) -> [String]
    {
        var messages: [String] = []
        add_issues(&messages, row.errors, true)
        if row.conflict == .list_item {
            messages.append(JOSA_FORMATTER.format(STRINGS.conflict_list_item, row.conflict_index))
        } else if row.conflict == .existing_item {
            messages.append(STRINGS.conflict_existing_item)
        }
        add_issues(&messages, row.warnings, false)
        if row.adjusted {
            messages.append(STRINGS.adjusted)
        }
        return messages
    }

    public static func issue_text(_ issue: NAME_ISSUE, error: Bool) -> String
    {
        switch issue {
        case .empty:
            return STRINGS.issue_error_empty
        case .dot_name:
            return STRINGS.issue_error_dot_name
        case .win_forbidden_char:
            return error
                ? STRINGS.issue_error_win_forbidden_char
                : STRINGS.issue_warning_win_forbidden_char
        case .win_reserved:
            return error ? STRINGS.issue_error_win_reserved : STRINGS.issue_warning_win_reserved
        case .win_trailing:
            return error ? STRINGS.issue_error_win_trailing : STRINGS.issue_warning_win_trailing
        case .win_name_too_long:
            return error
                ? STRINGS.issue_error_win_name_too_long
                : STRINGS.issue_warning_win_name_too_long
        case .win_path_too_long:
            return error
                ? STRINGS.issue_error_win_path_too_long
                : STRINGS.issue_warning_win_path_too_long
        case .mac_forbidden_char:
            return error
                ? STRINGS.issue_error_mac_forbidden_char
                : STRINGS.issue_warning_mac_forbidden_char
        case .mac_name_too_long:
            return error
                ? STRINGS.issue_error_mac_name_too_long
                : STRINGS.issue_warning_mac_name_too_long
        case .hidden:
            return STRINGS.issue_warning_hidden
        default:
            return ""
        }
    }

    public static func failure_text(_ failure: FAILURE, os_error: Int32) -> String
    {
        switch failure {
        case .in_use:
            return STRINGS.failure_in_use
        case .access_denied:
            return STRINGS.failure_access_denied
        case .not_found:
            return STRINGS.failure_not_found
        case .already_exists:
            return STRINGS.failure_already_exists
        case .replaced:
            return STRINGS.failure_replaced
        case .path_too_long:
            return STRINGS.failure_path_too_long
        case .depends_on_failed:
            return STRINGS.failure_depends_on_failed
        case .none, .other:
            return JOSA_FORMATTER.format(STRINGS.failure_other, os_error)
        }
    }

    public static func problem_text(_ problem: PROBLEM, kind: MOD_KIND, direct: Bool) -> String
    {
        switch problem.field {
        case .targets:
            return STRINGS.problem_targets
        case .find_text:
            return STRINGS.problem_find_text
        case .start_text:
            return STRINGS.problem_start_text
        case .end_text:
            return STRINGS.problem_end_text
        case .value where direct:
            return STRINGS.problem_value_direct
        case .value where kind == .insert:
            return STRINGS.problem_value_insert
        case .value:
            return STRINGS.problem_value_replace
        case .modified_date_format:
            return STRINGS.problem_modified_date_format
        case .created_date_format:
            return STRINGS.problem_created_date_format
        }
    }

    public static func index_error_text(_ error: INDEX_ERROR, item_count: UInt32) -> String
    {
        switch error {
        case .none:
            return ""
        case .syntax:
            return STRINGS.index_syntax
        case .out_of_range where item_count == 0:
            return STRINGS.index_empty_list
        case .out_of_range:
            return JOSA_FORMATTER.format(STRINGS.index_out_of_range, item_count)
        }
    }

    public static func example_state_text(_ state: EXAMPLE_STATE) -> String
    {
        switch state {
        case .skipped_dir:
            return STRINGS.example_skipped_dir
        case .no_target:
            return STRINGS.example_no_target
        case .changed, .unchanged:
            return ""
        }
    }

    public static func result_text(_ result: RESULT) -> String
    {
        return JOSA_FORMATTER.format(STRINGS.result_failed, result)
    }

    private static func add_issues(_ messages: inout [String], _ issues: NAME_ISSUE,
        _ error: Bool)
    {
        var bit: UInt32 = 1
        while bit != 0 {
            if issues.rawValue & bit != 0 {
                messages.append(issue_text(NAME_ISSUE(rawValue: bit), error: error))
            }
            bit <<= 1
        }
    }
}
