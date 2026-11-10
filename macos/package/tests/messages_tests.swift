import Testing

import limenamer_view_model

struct MESSAGES_TESTS
{
    @Test func lists_row_problems_in_order()
    {
        let row = ROW(item_id: 1, folder: "folder", name: "a.txt", new_name: ".a ", size: 0,
            modified_ms: 0, created_ms: 0, prefix_len: 0, suffix_len: 0,
            errors: [.win_forbidden_char, .win_trailing], warnings: .hidden, status: .error,
            conflict: .list_item, conflict_index: 3, is_dir: false, changed: true, adjusted: true,
            direct: false)

        #expect(MESSAGES.row_messages(row) == [
            STRINGS.issue_error_win_forbidden_char,
            STRINGS.issue_error_win_trailing,
            "3번 항목과 이름이 겹칩니다.",
            STRINGS.issue_warning_hidden,
            STRINGS.adjusted,
        ])
    }

    @Test func words_issues_by_severity()
    {
        #expect(MESSAGES.issue_text(.win_reserved, error: true)
            == "Windows 예약어(CON, NUL 등)라 쓸 수 없습니다.")
        #expect(MESSAGES.issue_text(.win_reserved, error: false)
            == "Windows에서는 예약어(CON, NUL 등)라 쓸 수 없습니다.")
    }

    @Test func formats_codes()
    {
        #expect(MESSAGES.status_name(.conflict) == "충돌")
        #expect(MESSAGES.failure_text(.other, os_error: 5) == "이름을 바꾸지 못했습니다. (오류 코드 5)")
        #expect(MESSAGES.index_error_text(.out_of_range, item_count: 12)
            == "번호는 1부터 12까지만 쓸 수 있습니다.")
        #expect(MESSAGES.index_error_text(.out_of_range, item_count: 0) == "목록이 비어 있습니다.")
        #expect(MESSAGES.example_state_text(.changed) == "")
        #expect(MESSAGES.result_text(.io) == JOSA_FORMATTER.format(STRINGS.result_failed, "io"))
    }

    @Test func words_value_problems_by_kind()
    {
        let problem = PROBLEM(field: .value, reason: .required)

        #expect(MESSAGES.problem_text(problem, kind: .insert, direct: false) == "넣을 값을 입력하세요.")
        #expect(MESSAGES.problem_text(problem, kind: .replace, direct: false) == "새 값을 입력하세요.")
        #expect(MESSAGES.problem_text(problem, kind: .replace, direct: true) == "새 이름을 입력하세요.")
    }
}
