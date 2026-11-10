import Darwin
import Testing

import limenamer_view_model

struct ENGINE_RENAME_TESTS
{
    @Test func renames_and_undoes_with_progress() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt"), try temp.file("b.txt")],
            mode: .as_items)
        _ = try engine.mod_add(TEST_MODS.INSERT_NEW)
        let progress = PROGRESS()

        let renamed: REPORT = try engine.rename(progress: progress)
        let renamed_on_disk: Bool = temp.exists("new_a.txt") && temp.exists("new_b.txt")
        let undo_state: (available: Bool, count: UInt32) = try engine.rename_undo_state()
        let checked_undo: REPORT = try engine.rename_undo_check()
        let undone: REPORT = try engine.rename_undo(progress: nil)

        #expect(renamed.total == 2)
        #expect(renamed.succeeded == 2)
        #expect(!renamed.needs_choice)
        #expect(renamed.problems.isEmpty)
        #expect(progress.read() == (2, 2))
        #expect(renamed_on_disk)
        #expect(undo_state == (true, 2))
        #expect(checked_undo.skipped == 0)
        #expect(undone.succeeded == 2)
        #expect(temp.exists("a.txt") && temp.exists("b.txt"))
        #expect(try engine.rename_undo_state() == (false, 0))
    }

    @Test func renames_on_a_background_task() async throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt")], mode: .as_items)
        _ = try engine.mod_add(TEST_MODS.INSERT_NEW)
        let progress = PROGRESS()

        let report: REPORT = try await Task.detached {
            return try engine.rename(progress: progress)
        }.value

        #expect(report.succeeded == 1)
        #expect(progress.read() == (1, 1))
        #expect(temp.exists("new_a.txt"))
    }

    @Test func refuses_to_rename_with_conflicts() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        _ = try temp.file("new_a.txt")
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt")], mode: .as_items)
        _ = try engine.mod_add(TEST_MODS.INSERT_NEW)

        #expect(throws: ENGINE_ERROR(result: .not_ready)) {
            try engine.rename(progress: nil)
        }
    }

    @Test func reports_failures_and_resolves() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        let a: String = try temp.file("a.txt")
        _ = try engine.items_add([a, try temp.file("b.txt")], mode: .as_items)
        let a_id: UInt64 = try engine.preview().rows[0].item_id
        _ = try engine.mod_add(TEST_MODS.INSERT_NEW)

        try #require(chflags(a, UInt32(UF_IMMUTABLE)) == 0)
        let failed: REPORT
        do {
            defer {
                chflags(a, 0)
            }
            failed = try engine.rename(progress: nil)
        }
        let resolved: REPORT = try engine.rename_resolve(.revert_all, progress: nil)

        #expect(failed.needs_choice)
        #expect(failed.failed == 1)
        try #require(failed.problems.count == 1)
        let problem: REPORT_ITEM = failed.problems[0]
        #expect(problem.item_id == a_id)
        #expect(problem.from_path == a)
        #expect(problem.to_path == temp.path + "/new_a.txt")
        #expect(problem.current_path == a)
        #expect(problem.stage == .run)
        #expect(problem.failure == .access_denied)
        #expect(problem.os_error != 0)
        #expect(!resolved.needs_choice)
        #expect(temp.exists("a.txt") && temp.exists("b.txt"))
        #expect(!temp.exists("new_b.txt"))
    }

    @Test func resolve_without_waiting_run_is_not_found() throws
    {
        let engine = try ENGINE()

        #expect(throws: ENGINE_ERROR(result: .not_found)) {
            try engine.rename_resolve(.keep_succeeded, progress: nil)
        }
    }
}
