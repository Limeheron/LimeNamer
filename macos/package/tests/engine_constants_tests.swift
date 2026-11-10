import Testing

import limenamer_engine
import limenamer_view_model

struct ENGINE_CONSTANTS_TESTS
{
    @Test func matches_c_enum_values()
    {
        #expect(Int(RESULT.internal_error.rawValue) == ln_result_internal)
        #expect(Int(RESULT.not_ready.rawValue) == ln_result_not_ready)
        #expect(Int(STATUS.conflict.rawValue) == ln_status_conflict)
        #expect(Int(CONFLICT.existing_item.rawValue) == ln_conflict_existing_item)
        #expect(Int(FAILURE.other.rawValue) == ln_failure_other)
        #expect(Int(STAGE.revert.rawValue) == ln_stage_revert)
        #expect(Int(MOD_FIELD.created_date_format.rawValue) == ln_mod_field_created_date_format)
        #expect(Int(PROBLEM_REASON.required.rawValue) == ln_problem_reason_required)
        #expect(Int(INDEX_ERROR.out_of_range.rawValue) == ln_index_error_out_of_range)
        #expect(Int(SORT_KEY.custom.rawValue) == ln_sort_key_custom)
        #expect(Int(ADD_MODE.folder_contents.rawValue) == ln_add_mode_folder_contents)
        #expect(Int(RESOLVE_CHOICE.keep_succeeded.rawValue) == ln_resolve_choice_keep_succeeded)
        #expect(Int(EXAMPLE_STATE.no_target.rawValue) == ln_example_state_no_target)
        #expect(Int(NAME_ISSUE.hidden.rawValue) == ln_name_issue_hidden)
        #expect(Int(NAME_ISSUE.mac_forbidden_char.rawValue) == ln_name_issue_mac_forbidden_char)
    }
}
