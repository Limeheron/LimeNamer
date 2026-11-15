import Testing

@testable import limenamer_view_model

@MainActor
struct MOD_EDITOR_VIEW_MODEL_TESTS
{
    @Test func reports_problems_on_apply_and_rechecks() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        scenario.mods.open_add()
        scenario.mods.add_kind = .replace
        scenario.mods.choose_position(.all)
        let editor: MOD_EDITOR_VIEW_MODEL = scenario.editor
        editor.position_type = .text

        let applied: Bool = try scenario.mods.apply()
        let find_problem: String = editor.problem(.find_text)
        editor.find_text = "a"

        #expect(!applied)
        #expect(scenario.mods.page == .edit)
        #expect(find_problem == "찾을 텍스트를 입력하세요.")
        #expect(editor.problem(.find_text) == "")
        #expect(editor.problem(.value) == "새 값을 입력하세요.")
        #expect(scenario.focused == .find_text)
        #expect(scenario.messages == [
            "적용하지 못했습니다. 찾을 텍스트를 입력하세요. 빨간 칸 2개를 확인하세요.",
        ])
    }

    @Test func builds_summary_and_example() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        scenario.mods.open_add()
        scenario.mods.add_kind = .insert
        scenario.mods.choose_position(.end)
        let editor: MOD_EDITOR_VIEW_MODEL = scenario.editor

        let initial: POSITION_TYPE = editor.position_type
        editor.value = [VALUE_PIECE(text: "_x")]

        #expect(initial == .end)
        #expect(editor.summary == "이름 맨 끝에 \"_x\" 넣기")
        #expect(editor.has_example)
        #expect(editor.example_index_text == "1번 항목")
        #expect(editor.example_before == ("a", "", ".txt"))
        #expect(editor.example_after == ("a", "_x", ".txt"))
        #expect(editor.example_state_text == "")
    }

    @Test func edits_mods_with_examples_before_them() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_OLD)
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods
        let editor: MOD_EDITOR_VIEW_MODEL = scenario.editor

        mods.open_card(mods.cards[1])
        let second_before: String = before_name(editor)
        mods.cancel()
        mods.open_card(mods.cards[0])
        let first_before: String = before_name(editor)
        editor.value = [VALUE_PIECE(text: "x_")]
        let applied: Bool = try mods.apply()

        #expect(second_before == "new_a.txt")
        #expect(first_before == "a.txt")
        #expect(applied)
        #expect(mods.cards[0].summary == "이름 맨 앞에 \"x_\" 넣기")
        #expect(try scenario.first_new_name() == "old_x_a.txt")
        #expect(scenario.messages == ["수정을 고쳤습니다."])
    }

    @Test func designates_targets_for_chosen_items() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt", "b.txt", "c.txt")
        defer {
            scenario.close()
        }
        let b: UInt64 = try scenario.items.id("b.txt")
        let list: ITEM_LIST_VIEW_MODEL = scenario.items.list
        list.set_selection([try scenario.items.id("a.txt")])
        scenario.mods.open_add()
        scenario.mods.add_kind = .insert
        scenario.mods.choose_position(.end)
        let editor: MOD_EDITOR_VIEW_MODEL = scenario.editor
        editor.value = [VALUE_PIECE(text: "_k")]

        editor.scope = .items
        let designating: Bool = list.designating
        let initial: Int = list.checked_ids.count
        list.toggle_check(b, range: false)
        let summary: String = editor.summary
        let applied: Bool = try scenario.mods.apply()

        #expect(designating)
        #expect(initial == 0)
        #expect(summary == "이름 맨 끝에 \"_k\" 넣기 (1개)")
        #expect(applied)
        #expect(!list.designating)
        #expect(try scenario.engine.preview().rows.map { $0.new_name }
            == ["a.txt", "b_k.txt", "c.txt"])
    }

    @Test func reports_target_problems() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt", "b.txt", "c.txt")
        defer {
            scenario.close()
        }
        let list: ITEM_LIST_VIEW_MODEL = scenario.items.list
        scenario.mods.open_add()
        scenario.mods.add_kind = .insert
        scenario.mods.choose_position(.end)
        let editor: MOD_EDITOR_VIEW_MODEL = scenario.editor
        editor.value = [VALUE_PIECE(text: "_k")]
        editor.scope = .items

        let applied: Bool = try scenario.mods.apply()
        let empty: String = editor.problem(.targets)
        list.number_text = "9"
        let range: String = editor.problem(.targets)
        list.toggle_check(try scenario.items.id("a.txt"), range: false)

        #expect(!applied)
        #expect(empty == "적용할 항목을 체크하거나 번호를 입력하세요.")
        #expect(range == "번호는 1부터 3까지만 쓸 수 있습니다.")
        #expect(editor.problem(.targets) == "")
        #expect(editor.summary == "이름 맨 끝에 \"_k\" 넣기 (1개)")
    }

    @Test func offers_choices_and_token_settings() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        let editor: MOD_EDITOR_VIEW_MODEL = scenario.editor
        scenario.mods.open_add()
        scenario.mods.add_kind = .delete
        scenario.mods.choose_position(.all)
        let delete_first: POSITION_TYPE = editor.position_tiles[0].type
        editor.position_type = .between
        let between_occurrences: Int = editor.occurrence_choices.count
        scenario.mods.cancel()

        scenario.mods.open_add()
        scenario.mods.add_kind = .insert
        scenario.mods.choose_position(.end)
        let insert_first: POSITION_TYPE = editor.position_tiles[0].type
        let before_tokens: Bool = editor.uses_number
        editor.value = [
            VALUE_PIECE(text: "a"),
            VALUE_PIECE(text: "b"),
            VALUE_PIECE(text: ""),
            VALUE_PIECE(token: .number),
            VALUE_PIECE(token: .modified_date),
        ]
        editor.modified_date_choice = ""
        editor.modified_date_custom = "yyyy"
        _ = try scenario.mods.apply()
        let stored: String = try scenario.engine.mods_get()

        #expect(delete_first == .all)
        #expect(between_occurrences == 4)
        #expect(insert_first == .start)
        #expect(!before_tokens)
        #expect(editor.value.count == 3)
        #expect(editor.uses_number && editor.uses_modified_date)
        #expect(stored.contains(#""text":"ab""#), "\(stored)")
        #expect(stored.contains(#""digits":3"#), "\(stored)")
        #expect(stored.contains(#""format":"yyyy""#), "\(stored)")
    }

    @Test func offers_position_samples() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        let editor: MOD_EDITOR_VIEW_MODEL = scenario.editor
        let insert: [POSITION_CHOICE] = editor.position_choices(.insert)
        let delete: [POSITION_CHOICE] = editor.position_choices(.delete)
        let replace: [POSITION_CHOICE] = editor.position_choices(.replace)
        let format: [POSITION_CHOICE] = editor.position_choices(.format)

        #expect(insert.map { $0.type } == [.start, .end, .after_nth, .before_nth_from_end,
            .before_text, .after_text])
        #expect(delete.map { $0.type } == [.all, .from_nth, .first_n, .last_n, .text, .between])
        #expect(insert[0].label == "맨 앞")
        #expect(insert[0].description == "이름 앞에 붙입니다.")
        #expect(insert[2].label == "앞에서 N글자 뒤")
        #expect(insert[0].after == [piece("2024_", .mark), piece("IMG_0001.JPG", .plain)])
        #expect(insert[0].tile == [piece("", .caret), piece("IMG_0001.JPG", .plain)])
        #expect(insert[1].tile == [piece("IMG_0001", .plain), piece("", .caret),
            piece(".JPG", .plain)])
        #expect(insert[4].tile == [piece("IMG_", .plain), piece("", .caret),
            piece("0001", .anchor), piece(".JPG", .plain)])
        #expect(insert[5].tile == [piece("IMG", .anchor), piece("", .caret),
            piece("_0001.JPG", .plain)])
        #expect(delete[3].before == [piece("IMG_", .plain), piece("0001", .removed),
            piece(".JPG", .plain)])
        #expect(delete[3].after == [piece("IMG_.JPG", .plain)])
        #expect(delete[3].tile == [piece("IMG_", .plain), piece("0001", .mark),
            piece(".JPG", .plain)])
        #expect(delete[1].tile == [piece("I", .plain), piece("MG_", .mark),
            piece("0001.JPG", .plain)])
        #expect(delete[4].tile == [piece("IMG_", .mark_anchor), piece("0001.JPG", .plain)])
        #expect(delete[5].tile == [piece("IMG", .anchor), piece("_0001", .mark),
            piece(".JPG", .plain)])
        #expect(replace[0].after == [piece("제주_", .mark), piece(".JPG", .plain)])
        #expect(!format[0].shows_after)
        #expect(format[3].before == format[3].tile)
    }

    @Test func starts_inserts_at_the_front() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        scenario.mods.open_add()
        scenario.mods.choose_position(.start)
        scenario.editor.value = [VALUE_PIECE(text: "x_")]
        _ = try scenario.mods.apply()

        #expect(try scenario.first_new_name() == "x_a.txt")
        #expect(scenario.editor.position_choices(.insert)[0].type == .start)
    }

    private func piece(_ text: String, _ style: SAMPLE_STYLE) -> SAMPLE_PIECE
    {
        return SAMPLE_PIECE(text: text, style: style)
    }

    private func before_name(_ editor: MOD_EDITOR_VIEW_MODEL) -> String
    {
        let parts: (head: String, mark: String, tail: String) = editor.example_before
        return parts.head + parts.mark + parts.tail
    }
}
