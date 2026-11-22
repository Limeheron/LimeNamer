import Darwin
import Foundation
import Testing

@testable import limenamer_view_model

@MainActor
struct MAIN_VIEW_MODEL_TESTS
{
    @Test func adds_removes_and_clears_items() throws
    {
        let scenario = try MAIN_SCENARIO()
        defer {
            scenario.close()
        }
        let main: MAIN_VIEW_MODEL = scenario.main
        let a: String = try scenario.temp.file("a.txt")
        let b: String = try scenario.temp.file("b.txt")

        main.add_paths([a, b, a, scenario.temp.path + "/bad\u{0}.txt"], mode: .as_items)
        let added: String = scenario.message
        let could_remove: Bool = main.can_remove
        main.list.set_selection([try scenario.id("a.txt")])
        let can_remove: Bool = main.can_remove
        main.remove_selected()
        let removed: String = scenario.message
        main.undo()
        let restored: Int = main.list.count_all
        main.redo()
        main.clear()

        #expect(added == "2개 항목을 추가했습니다. 이미 목록에 있는 1개는 건너뛰었습니다. "
            + "1개는 추가하지 못했습니다.")
        #expect(!could_remove)
        #expect(can_remove)
        #expect(removed == "1개 항목을 목록에서 뺐습니다. 실행 취소로 되돌릴 수 있습니다.")
        #expect(restored == 2)
        #expect(main.list.count_all == 0)
        #expect(scenario.message == "목록을 비웠습니다. 실행 취소로 되돌릴 수 있습니다.")
        #expect(!main.can_clear)
    }

    @Test func blocks_actions_while_editing() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        let main: MAIN_VIEW_MODEL = scenario.main
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        main.mods.open_add()
        main.mods.add_kind = .insert
        main.mods.choose_position(.end)
        let block: String = "먼저 편집을 적용하거나 취소하세요."

        main.undo()
        let undo: String = scenario.message
        main.panel_open = false
        let panel: String = scenario.message
        main.direct_edit(try scenario.id("a.txt"), new_name: "b.txt")
        let direct: String = scenario.message
        await main.rename()

        #expect(main.is_editing)
        #expect(undo == "편집 중에는 실행 취소할 수 없습니다. " + block)
        #expect(panel == "편집 중에는 수정 패널을 닫을 수 없습니다. " + block)
        #expect(main.panel_open)
        #expect(direct == "편집 중에는 이름을 직접 수정할 수 없습니다. " + block)
        #expect(scenario.message == "편집 중에는 이름을 바꿀 수 없습니다. " + block)
        #expect(scenario.temp.exists("a.txt"))
    }

    @Test func renames_after_confirming_warnings() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        let main: MAIN_VIEW_MODEL = scenario.main
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        main.direct_edit(try scenario.id("b.txt"), new_name: ".b")
        main.list.filter = .warning
        scenario.dialogs.answers = [false]

        await main.rename()
        let kept: Bool = scenario.temp.exists("a.txt")
        await main.rename()

        try #require(scenario.dialogs.asked.count == 2)
        let warning: DIALOG_CONTENT = scenario.dialogs.asked[0]
        #expect(kept)
        #expect(warning.kind == .confirm)
        #expect(warning.title == "경고가 있는 항목이 있습니다")
        #expect(warning.message == "경고 1개를 포함해 2개 항목의 이름을 바꿉니다.")
        #expect(warning.lines == [DIALOG_LINE(icon: .warning, index: 2, text: "b.txt → .b",
            reason: "점으로 시작해 macOS에서 숨김 파일이 됩니다.")])
        #expect(scenario.temp.exists("new_a.txt") && scenario.temp.exists(".b"))
        #expect(scenario.message == "2개 항목의 이름을 바꿨습니다. "
            + "다시 적용되지 않게 수정을 모두 껐습니다.")
        #expect(main.list.filter == .all)
        #expect(main.can_rename_undo)
        #expect(main.mods.all_enabled_state == false)
    }

    @Test func refuses_to_rename_with_problems() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        _ = try scenario.temp.file("new_b.txt")
        scenario.main.activated()
        try scenario.add_mod(TEST_MODS.INSERT_NEW)

        await scenario.main.rename()

        #expect(scenario.message == "충돌 1개가 있어 이름을 바꾸지 않았습니다. "
            + "첫 문제는 2번 항목입니다.")
        #expect(scenario.main.list.selected_ids == [try scenario.id("b.txt")])
        #expect(scenario.temp.exists("a.txt"))
    }

    @Test func keeps_succeeded_items_after_failure() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        scenario.dialogs.answers = [false]

        try await rename_locked(scenario, "a.txt")

        try #require(scenario.dialogs.asked.count == 1)
        let choice: DIALOG_CONTENT = scenario.dialogs.asked[0]
        #expect(choice.kind == .choice)
        #expect(choice.title == "일부 항목의 이름을 바꾸지 못했습니다")
        #expect(choice.message
            == "2개 중 1개를 바꿨고 1개는 바꾸지 못했습니다. 처리 방식을 고르세요.")
        #expect(choice.lines == [DIALOG_LINE(icon: .error, index: 1,
            text: "a.txt → new_a.txt", reason: "접근 권한이 없습니다.")])
        #expect(choice.primary == "전체 되돌리기")
        #expect(choice.secondary == "성공한 항목 유지")
        #expect(scenario.message == "1개 항목의 이름을 바꿨습니다. 실패한 1개는 그대로 있습니다. "
            + "다시 적용되지 않게 수정을 모두 껐습니다.")
        #expect(scenario.temp.exists("a.txt") && scenario.temp.exists("new_b.txt"))
    }

    @Test func reverts_all_after_failure() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        scenario.dialogs.answers = [true]

        try await rename_locked(scenario, "a.txt")

        #expect(scenario.message == "모든 항목을 실행 전 이름으로 되돌렸습니다.")
        #expect(scenario.temp.exists("a.txt") && scenario.temp.exists("b.txt"))
        #expect(!scenario.main.can_rename_undo)
        #expect(scenario.main.mods.all_enabled_state == true)
    }

    @Test func reports_items_that_all_failed() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)

        try await rename_locked(scenario, "a.txt")

        try #require(scenario.dialogs.asked.count == 1)
        let report: DIALOG_CONTENT = scenario.dialogs.asked[0]
        #expect(report.kind == .report)
        #expect(report.title == "이름을 바꾸지 못했습니다")
        #expect(report.message == "1개 항목의 이름을 바꾸지 못했습니다.")
        #expect(scenario.message == "1개 항목의 이름을 바꾸지 못했습니다.")
        #expect(scenario.temp.exists("a.txt"))
    }

    @Test func reports_items_left_after_revert() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        let renamed: String = scenario.temp.path + "/new_b.txt"
        scenario.dialogs.on_ask = { _ in
            chflags(renamed, UInt32(UF_IMMUTABLE))
        }
        defer {
            chflags(renamed, 0)
        }

        try await rename_locked(scenario, "a.txt")

        try #require(scenario.dialogs.asked.count == 2)
        #expect(scenario.dialogs.asked[1].title == "일부 항목을 원래 이름으로 돌려놓지 못했습니다")
        #expect(scenario.message
            == "되돌리지 못한 1개를 빼고 모든 항목을 실행 전 이름으로 되돌렸습니다.")
        #expect(scenario.temp.exists("a.txt") && scenario.temp.exists("new_b.txt"))
    }

    @Test func skips_folder_refresh_while_choosing() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        scenario.dialogs.answers = [false]
        var message: String = ""
        scenario.dialogs.on_ask = { _ in
            scenario.main.activated()
            message = scenario.message
        }

        try await rename_locked(scenario, "a.txt")

        #expect(message == "2개 중 1개 항목의 이름을 바꾸지 못했습니다. 처리 방식을 고르세요.")
    }

    @Test func blocks_close_while_choosing() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        scenario.dialogs.answers = [false]
        var allowed: Bool = true
        var message: String = ""
        scenario.dialogs.on_ask = { _ in
            allowed = scenario.main.allow_close()
            message = scenario.message
        }

        try await rename_locked(scenario, "a.txt")

        #expect(!allowed)
        #expect(message == "이름을 바꾸거나 되돌리는 중에는 닫을 수 없습니다.")
        #expect(scenario.main.allow_close())
    }

    @Test func reveals_problems_found_when_running() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        _ = try scenario.temp.file("new_a.txt")

        await scenario.main.rename()

        let id: UInt64 = try scenario.id("a.txt")
        #expect(scenario.message
            == "폴더 내용이 바뀌어 다시 검사했습니다. 오류나 충돌을 해결한 뒤 다시 하세요.")
        #expect(scenario.main.list.selected_ids == [id])
        #expect(scenario.temp.exists("a.txt"))
    }

    @Test func undoes_rename_after_confirmation() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        await scenario.main.rename()
        try FileManager.default.removeItem(atPath: scenario.temp.path + "/new_b.txt")

        await scenario.main.rename_undo()

        try #require(scenario.dialogs.asked.count == 2)
        let confirm: DIALOG_CONTENT = scenario.dialogs.asked[0]
        let result: DIALOG_CONTENT = scenario.dialogs.asked[1]
        #expect(confirm.title == "마지막으로 바꾼 이름을 되돌릴까요?")
        #expect(confirm.message == "1개 항목을 바꾸기 전 이름으로 돌립니다. 목록을 고쳤더라도 "
            + "마지막 실행 기록대로 되돌립니다. 아래 1개는 되돌릴 수 없어 건너뜁니다.")
        #expect(confirm.lines == [DIALOG_LINE(icon: .warning, index: 2,
            text: "new_b.txt → b.txt", reason: "항목을 찾을 수 없습니다.")])
        #expect(result.kind == .report)
        #expect(result.title == "일부 항목을 되돌리지 못했습니다")
        #expect(scenario.message == "1개 항목을 실행 전 이름으로 되돌렸습니다. "
            + "건너뛴 1개는 되돌리지 않았습니다.")
        #expect(scenario.temp.exists("a.txt"))
    }

    @Test func reports_undo_failures() async throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        try scenario.add_mod(TEST_MODS.INSERT_NEW)
        await scenario.main.rename()
        let renamed: String = scenario.temp.path + "/new_a.txt"
        try #require(chflags(renamed, UInt32(UF_IMMUTABLE)) == 0)
        defer {
            chflags(renamed, 0)
        }

        await scenario.main.rename_undo()

        try #require(scenario.dialogs.asked.count == 2)
        #expect(scenario.dialogs.asked[1].title == "일부 항목을 되돌리지 못했습니다")
        #expect(scenario.message
            == "1개 항목을 실행 전 이름으로 되돌렸습니다. 실패한 1개는 그대로 있습니다.")
        #expect(scenario.temp.exists("new_a.txt") && scenario.temp.exists("b.txt"))
    }

    @Test func sorts_and_moves_items() throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt", "c.txt")
        defer {
            scenario.close()
        }
        let main: MAIN_VIEW_MODEL = scenario.main

        main.sort(.name_natural, descending: true)
        let sorted: (key: SORT_KEY, descending: Bool) = (main.sort_key, main.sort_descending)
        main.move_rows([try scenario.id("c.txt")], target_id: try scenario.id("a.txt"),
            after: true)

        #expect(sorted == (.name_natural, true))
        #expect(main.list.rows.map { $0.name } == ["b.txt", "a.txt", "c.txt"])
        #expect(main.sort_key == .custom)
        #expect(scenario.message == "순서를 바꿨습니다. 정렬은 사용자 지정이 됩니다.")
    }

    @Test func edits_directly_and_reports_rows() throws
    {
        let scenario = try MAIN_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        let main: MAIN_VIEW_MODEL = scenario.main
        let b: UInt64 = try scenario.id("b.txt")

        main.direct_edit(b, new_name: "b:.txt")
        let edited: String = scenario.message
        main.show_row_problems(b)
        let problems: String = scenario.message
        try FileManager.default.removeItem(atPath: scenario.temp.path + "/a.txt")
        let missing: String? = main.location_of(try scenario.id("a.txt"))

        #expect(edited == "2번 항목을 직접 수정했습니다.")
        #expect(problems == "2번 항목: macOS에서 쓸 수 없는 문자(/ :)가 있습니다. "
            + "Windows에서는 쓸 수 없는 문자가 있습니다.")
        #expect(missing == nil)
        #expect(scenario.message == "항목을 찾을 수 없습니다.")
        #expect(main.location_of(b) == scenario.temp.path + "/b.txt")
    }

    @Test func applies_duplicate_options() throws
    {
        let scenario = try MAIN_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        let main: MAIN_VIEW_MODEL = scenario.main

        main.duplicate_enabled = true
        main.duplicate_format = .custom
        let custom: String = try scenario.engine.options_get()
        main.duplicate_custom_text = ""
        let unchanged: String = try scenario.engine.options_get()
        main.duplicate_custom_text = String(repeating: "가", count: 33)
        let too_long: String = try scenario.engine.options_get()
        main.duplicate_custom_text = "_v"

        #expect(main.duplicate_enabled)
        #expect(custom.contains(#""custom_text":"_copy""#), "\(custom)")
        #expect(unchanged == custom)
        #expect(too_long == custom)
        #expect(main.duplicate_format == .custom)
        #expect(try scenario.engine.options_get().contains(#""custom_text":"_v""#))
    }

    @Test func toggles_theme_and_keeps_settings() throws
    {
        let scenario = try MAIN_SCENARIO()
        defer {
            scenario.close()
        }
        let main: MAIN_VIEW_MODEL = scenario.main

        main.toggle_theme(system_dark: true)
        let light: String = scenario.message
        main.toggle_theme(system_dark: true)
        scenario.settings.show_size = true
        scenario.settings.example_collapsed = true
        let loaded = SETTINGS(store: scenario.store)
        let broken_store = MEMORY_STORE()
        broken_store.values = ["theme_mode": "sepia", "show_size": "yes", "show_index": 0.5]
        let broken = SETTINGS(store: broken_store)

        #expect(light == "화면 모드를 밝게로 바꿨습니다.")
        #expect(scenario.message == "화면 모드를 어둡게로 바꿨습니다.")
        #expect(loaded.theme_mode == .dark)
        #expect(loaded.show_size && loaded.example_collapsed && loaded.show_index)
        #expect(broken.theme_mode == .system)
        #expect(!broken.show_size)
        #expect(broken.show_index)
    }

    private func rename_locked(_ scenario: MAIN_SCENARIO, _ name: String) async throws
    {
        let path: String = scenario.temp.path + "/" + name
        try #require(chflags(path, UInt32(UF_IMMUTABLE)) == 0)
        defer {
            chflags(path, 0)
        }
        await scenario.main.rename()
    }
}
