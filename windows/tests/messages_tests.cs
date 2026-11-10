using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class MESSAGES_TESTS
{
    [TestMethod]
    public void lists_row_problems_in_order()
    {
        ROW row = new(1, "folder", "a.txt", ".a ", 0, 0, 0, 0, 0,
            NAME_ISSUE.win_forbidden_char | NAME_ISSUE.win_trailing, NAME_ISSUE.hidden,
            STATUS.error, CONFLICT.list_item, 3, false, true, true, false);

        CollectionAssert.AreEqual(new[] {
            STRINGS.issue_error_win_forbidden_char,
            STRINGS.issue_error_win_trailing,
            "3번 항목과 이름이 겹칩니다.",
            STRINGS.issue_warning_hidden,
            STRINGS.adjusted,
        }, (System.Collections.ICollection)MESSAGES.row_messages(row));
    }

    [TestMethod]
    public void words_issues_by_severity()
    {
        Assert.AreEqual("Windows 예약어(CON, NUL 등)라 쓸 수 없습니다.",
            MESSAGES.issue_text(NAME_ISSUE.win_reserved, true));
        Assert.AreEqual("Windows에서는 예약어(CON, NUL 등)라 쓸 수 없습니다.",
            MESSAGES.issue_text(NAME_ISSUE.win_reserved, false));
    }

    [TestMethod]
    public void formats_codes()
    {
        Assert.AreEqual("충돌", MESSAGES.status_name(STATUS.conflict));
        Assert.AreEqual("이름을 바꾸지 못했습니다. (오류 코드 5)",
            MESSAGES.failure_text(FAILURE.other, 5));
        Assert.AreEqual("번호는 1부터 12까지만 쓸 수 있습니다.",
            MESSAGES.index_error_text(INDEX_ERROR.out_of_range, 12));
        Assert.AreEqual("목록이 비어 있습니다.",
            MESSAGES.index_error_text(INDEX_ERROR.out_of_range, 0));
        Assert.AreEqual("", MESSAGES.example_state_text(EXAMPLE_STATE.changed));
    }

    [TestMethod]
    public void words_value_problems_by_kind()
    {
        PROBLEM problem = new(MOD_FIELD.value, PROBLEM_REASON.required);

        Assert.AreEqual("넣을 값을 입력하세요.",
            MESSAGES.problem_text(problem, MOD_KIND.insert, false));
        Assert.AreEqual("새 값을 입력하세요.",
            MESSAGES.problem_text(problem, MOD_KIND.replace, false));
        Assert.AreEqual("새 이름을 입력하세요.",
            MESSAGES.problem_text(problem, MOD_KIND.replace, true));
    }
}
