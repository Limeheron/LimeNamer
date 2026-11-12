import Testing

@testable import limenamer_view_model

@MainActor
struct ITEM_LIST_VIEW_MODEL_TESTS
{
    @Test func builds_rows_from_preview() throws
    {
        let scenario = try LIST_SCENARIO()
        defer {
            scenario.close()
        }
        let a: String = try scenario.temp.file("a.txt", "abc")
        let photos: String = try scenario.temp.folder("photos")
        _ = try scenario.engine.items_add([a, photos], mode: .as_items)
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        try scenario.load()

        try #require(scenario.list.rows.count == 2)
        let file: ITEM_ROW = scenario.list.rows[0]
        let folder: ITEM_ROW = scenario.list.rows[1]
        #expect(file.index == 1)
        #expect(file.full_path == a)
        #expect(file.new_name_parts == ("", "new_", "a.txt"))
        #expect(file.status_text == "정상")
        #expect(file.size_text == "3B")
        #expect(file.modified_text.wholeMatch(of: /\d{4}-\d{2}-\d{2} \d{2}:\d{2}/) != nil)
        #expect(folder.is_dir)
        #expect(folder.size_text == "—")
        #expect(scenario.list.header_text == "2개 · 선택 0개")
    }

    @Test func filters_rows_by_status() throws
    {
        let scenario = try STATUS_SCENARIO.create()
        defer {
            scenario.close()
        }
        let list: ITEM_LIST_VIEW_MODEL = scenario.list

        #expect((list.count_changed, list.count_error, list.count_conflict, list.count_warning)
            == (4, 1, 1, 1))
        list.filter = .error
        #expect(scenario.visible_names() == ["c.txt"])
        #expect(!list.can_drag)
        #expect(list.header_text == "5개 · 선택 0개 · 오류 항목만 보는 중")
        #expect(list.subtitle_text == "항목 5개 · 선택 0개 · 오류 항목만 보는 중")
        list.filter = .conflict
        #expect(scenario.visible_names() == ["b.txt"])
        list.filter = .warning
        #expect(scenario.visible_names() == ["d.txt"])
        list.filter = .changed
        #expect(scenario.visible_names() == ["a.txt", "b.txt", "c.txt", "d.txt"])
    }

    @Test func reports_empty_states() throws
    {
        let empty = try LIST_SCENARIO()
        let scenario = try LIST_SCENARIO("a.txt")
        defer {
            empty.close()
            scenario.close()
        }

        let empty_list: Bool = empty.list.shows_empty_list
        let filled: Bool = scenario.list.shows_empty_list || scenario.list.shows_empty_filter
        scenario.list.filter = .error

        #expect(empty_list)
        #expect(!empty.list.shows_empty_filter)
        #expect(!filled)
        #expect(scenario.list.shows_empty_filter)
        #expect(scenario.list.empty_filter_title == "오류 항목이 없습니다")
    }

    @Test func keeps_selection_across_loads() throws
    {
        let scenario = try LIST_SCENARIO("a.txt", "b.txt")
        defer {
            scenario.close()
        }
        let a: UInt64 = try scenario.id("a.txt")
        let b: UInt64 = try scenario.id("b.txt")
        var requested: Set<UInt64> = []
        scenario.list.selection_requested = { requested = $0 }

        scenario.list.set_selection([a, b])
        try scenario.engine.items_remove([a])
        try scenario.load()

        #expect(scenario.list.selected_ids == [b])
        #expect(requested == [b])
    }

    @Test func reveals_hidden_rows() throws
    {
        let scenario = try STATUS_SCENARIO.create()
        defer {
            scenario.close()
        }
        let a: UInt64 = try scenario.id("a.txt")
        var revealed: UInt64 = 0
        scenario.list.reveal_requested = { revealed = $0 }

        scenario.list.filter = .error
        scenario.list.reveal(a)

        #expect(scenario.list.filter == .all)
        #expect(scenario.list.selected_ids == [a])
        #expect(revealed == a)
    }

    @Test func moves_dragged_rows() throws
    {
        let scenario = try LIST_SCENARIO("a.txt", "b.txt", "c.txt", "d.txt")
        defer {
            scenario.close()
        }
        let a: UInt64 = try scenario.id("a.txt")
        let b: UInt64 = try scenario.id("b.txt")
        let c: UInt64 = try scenario.id("c.txt")
        let d: UInt64 = try scenario.id("d.txt")
        let list: ITEM_LIST_VIEW_MODEL = scenario.list

        list.set_selection([c, a])

        #expect(list.drag_ids(c) == [a, c])
        #expect(list.drag_ids(b) == [b])
        #expect(!list.can_drop([b], target_id: a, after: true))
        #expect(!list.can_drop([a, c], target_id: c, after: true))
        #expect(try list.drop([a, c], target_id: d, after: true))
        try scenario.load()
        #expect(scenario.visible_names() == ["b.txt", "d.txt", "a.txt", "c.txt"])
        list.filter = .changed
        #expect(!list.can_drop([b], target_id: d, after: true))
    }

    @Test func designates_with_checks_and_numbers() throws
    {
        let scenario = try LIST_SCENARIO("a.txt", "b.txt", "c.txt", "d.txt", "e.txt")
        defer {
            scenario.close()
        }
        let list: ITEM_LIST_VIEW_MODEL = scenario.list
        let a: UInt64 = try scenario.id("a.txt")
        let d: UInt64 = try scenario.id("d.txt")

        list.begin_designation([try scenario.id("b.txt")])
        let initial: String = list.number_text
        let a_dimmed: Bool = list.rows[0].is_dimmed
        list.toggle_check(a, range: false)
        list.toggle_check(d, range: true)
        let ranged: String = list.number_text
        let partial: Bool? = list.check_all_state
        list.number_text = "2, 4-5"
        let typed: [String] = checked_names(list)
        list.number_text = "9"
        let error: String = list.number_error
        let kept: [String] = checked_names(list)
        list.check_all(true)

        #expect(initial == "2")
        #expect(a_dimmed)
        #expect(ranged == "1-4")
        #expect(partial == nil)
        #expect(typed == ["b.txt", "d.txt", "e.txt"])
        #expect(error == "번호는 1부터 5까지만 쓸 수 있습니다.")
        #expect(kept == typed)
        #expect(list.number_text == "1-5")
        #expect(list.check_all_state == true)
        #expect(list.header_text == "5개 · 체크 5개")
        list.end_designation()
        #expect(!list.rows.contains { $0.is_dimmed || $0.is_checked })
        #expect(list.number_text == "")
    }

    @Test func renumbers_designation_after_reorder() throws
    {
        let scenario = try LIST_SCENARIO("a.txt", "b.txt", "c.txt", "d.txt", "e.txt")
        defer {
            scenario.close()
        }

        scenario.list.begin_designation([try scenario.id("a.txt"), try scenario.id("b.txt")])
        let before: String = scenario.list.number_text
        try scenario.engine.items_sort(.name_natural, descending: true)
        try scenario.load()

        #expect(before == "1, 2")
        #expect(scenario.list.number_text == "4, 5")
    }

    private func checked_names(_ list: ITEM_LIST_VIEW_MODEL) -> [String]
    {
        return list.rows.filter { $0.is_checked }.map { $0.name }
    }
}
