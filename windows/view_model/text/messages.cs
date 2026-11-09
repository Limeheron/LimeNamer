using System.Collections.Generic;

namespace limenamer;

public static class MESSAGES
{
    public static string status_name(STATUS status)
    {
        return status switch {
            STATUS.ok => STRINGS.status_ok,
            STATUS.unchanged => STRINGS.status_unchanged,
            STATUS.warning => STRINGS.status_warning,
            STATUS.error => STRINGS.status_error,
            _ => STRINGS.status_conflict,
        };
    }

    public static IReadOnlyList<string> row_messages(ROW row)
    {
        List<string> messages = new();
        add_issues(messages, row.errors, true);
        if (row.conflict == CONFLICT.list_item) {
            messages.Add(JOSA_FORMATTER.format(STRINGS.conflict_list_item, row.conflict_index));
        } else if (row.conflict == CONFLICT.existing_item) {
            messages.Add(STRINGS.conflict_existing_item);
        }
        add_issues(messages, row.warnings, false);
        if (row.adjusted) {
            messages.Add(STRINGS.adjusted);
        }
        return messages;
    }

    public static string issue_text(NAME_ISSUE issue, bool error)
    {
        return issue switch {
            NAME_ISSUE.empty => STRINGS.issue_error_empty,
            NAME_ISSUE.dot_name => STRINGS.issue_error_dot_name,
            NAME_ISSUE.win_forbidden_char => error
                ? STRINGS.issue_error_win_forbidden_char
                : STRINGS.issue_warning_win_forbidden_char,
            NAME_ISSUE.win_reserved => error
                ? STRINGS.issue_error_win_reserved
                : STRINGS.issue_warning_win_reserved,
            NAME_ISSUE.win_trailing => error
                ? STRINGS.issue_error_win_trailing
                : STRINGS.issue_warning_win_trailing,
            NAME_ISSUE.win_name_too_long => error
                ? STRINGS.issue_error_win_name_too_long
                : STRINGS.issue_warning_win_name_too_long,
            NAME_ISSUE.win_path_too_long => error
                ? STRINGS.issue_error_win_path_too_long
                : STRINGS.issue_warning_win_path_too_long,
            NAME_ISSUE.mac_forbidden_char => error
                ? STRINGS.issue_error_mac_forbidden_char
                : STRINGS.issue_warning_mac_forbidden_char,
            NAME_ISSUE.mac_name_too_long => error
                ? STRINGS.issue_error_mac_name_too_long
                : STRINGS.issue_warning_mac_name_too_long,
            NAME_ISSUE.hidden => STRINGS.issue_warning_hidden,
            _ => "",
        };
    }

    public static string failure_text(FAILURE failure, int os_error)
    {
        return failure switch {
            FAILURE.in_use => STRINGS.failure_in_use,
            FAILURE.access_denied => STRINGS.failure_access_denied,
            FAILURE.not_found => STRINGS.failure_not_found,
            FAILURE.already_exists => STRINGS.failure_already_exists,
            FAILURE.replaced => STRINGS.failure_replaced,
            FAILURE.path_too_long => STRINGS.failure_path_too_long,
            FAILURE.depends_on_failed => STRINGS.failure_depends_on_failed,
            _ => JOSA_FORMATTER.format(STRINGS.failure_other, os_error),
        };
    }

    public static string problem_text(PROBLEM problem, MOD_KIND kind, bool direct)
    {
        return problem.field switch {
            MOD_FIELD.targets => STRINGS.problem_targets,
            MOD_FIELD.find_text => STRINGS.problem_find_text,
            MOD_FIELD.start_text => STRINGS.problem_start_text,
            MOD_FIELD.end_text => STRINGS.problem_end_text,
            MOD_FIELD.value when direct => STRINGS.problem_value_direct,
            MOD_FIELD.value when kind == MOD_KIND.insert => STRINGS.problem_value_insert,
            MOD_FIELD.value => STRINGS.problem_value_replace,
            MOD_FIELD.modified_date_format => STRINGS.problem_modified_date_format,
            _ => STRINGS.problem_created_date_format,
        };
    }

    public static string index_error_text(INDEX_ERROR error, uint item_count)
    {
        return error switch {
            INDEX_ERROR.none => "",
            INDEX_ERROR.syntax => STRINGS.index_syntax,
            _ when item_count == 0 => STRINGS.index_empty_list,
            _ => JOSA_FORMATTER.format(STRINGS.index_out_of_range, item_count),
        };
    }

    public static string example_state_text(EXAMPLE_STATE state)
    {
        return state switch {
            EXAMPLE_STATE.skipped_dir => STRINGS.example_skipped_dir,
            EXAMPLE_STATE.no_target => STRINGS.example_no_target,
            _ => "",
        };
    }

    public static string result_text(RESULT result)
    {
        return JOSA_FORMATTER.format(STRINGS.result_failed, result);
    }

    private static void add_issues(List<string> messages, NAME_ISSUE issues, bool error)
    {
        for (uint bit = 1; bit != 0; bit <<= 1) {
            if (((uint)issues & bit) != 0) {
                messages.Add(issue_text((NAME_ISSUE)bit, error));
            }
        }
    }
}
