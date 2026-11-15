import Testing

@testable import limenamer_view_model

@MainActor
struct MOD_LIST_VIEW_MODEL_TESTS
{
    @Test func loads_cards_with_summaries() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt", "b.txt", "c.txt")
        defer {
            scenario.close()
        }
        let a: UInt64 = try scenario.items.id("a.txt")
        let c: UInt64 = try scenario.items.id("c.txt")
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        try scenario.engine.mod_direct_edit(try scenario.items.id("b.txt"), new_name: "x.txt")
        _ = try scenario.engine.mod_add("""
            {"kind":"insert","position":{"type":"end"},"value":[{"text":"_k"}],\
            "scope":"items","targets":[\(a),\(c)]}
            """)
        try scenario.engine.items_remove([c])

        try scenario.load()

        let mods: MOD_LIST_VIEW_MODEL = scenario.mods
        #expect(mods.cards.map { $0.summary } == [
            "이름 맨 앞에 \"new_\" 넣기",
            "2번 항목을 \"x.txt\"로 직접 수정",
            "이름 맨 끝에 \"_k\" 넣기 (1개)",
        ])
        try #require(mods.cards.count == 3)
        #expect(mods.cards[1].direct)
        #expect(mods.all_enabled_state == true)
        #expect(mods.enabled_count == 3)
    }

    @Test func toggles_and_removes_mods() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_OLD)
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        try mods.set_enabled(mods.cards[0], enabled: false)
        let mixed: Bool? = mods.all_enabled_state
        try mods.toggle_all()
        let all_on: Bool? = mods.all_enabled_state
        try mods.toggle_all()
        let all_off: Bool? = mods.all_enabled_state
        try mods.remove(mods.cards[1])

        #expect(mixed == nil)
        #expect(all_on == true)
        #expect(all_off == false)
        #expect(mods.cards.count == 1)
        #expect(scenario.messages == [
            "수정을 껐습니다.",
            "모든 수정을 켰습니다.",
            "모든 수정을 껐습니다.",
            "수정을 지웠습니다. 실행 취소로 되돌릴 수 있습니다.",
        ])
    }

    @Test func reorders_cards() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        let first: UInt64 = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        let second: UInt64 = try scenario.engine.mod_add(TEST_MODS.INSERT_OLD)
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        #expect(!mods.can_drop(first, target_id: first, after: true))
        #expect(!mods.can_drop(first, target_id: second, after: false))
        #expect(try mods.drop(second, target_id: first, after: false))

        #expect(mods.cards.map { $0.id } == [second, first])
        #expect(try scenario.first_new_name() == "new_old_a.txt")
        #expect(scenario.messages == ["수정 순서를 바꿨습니다."])
    }

    @Test func adds_mod_from_add_page() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.JPG")
        defer {
            scenario.close()
        }
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        mods.open_add()
        let insert: KIND_CARD = mods.kind_header
        let badge: String = mods.badge_text
        mods.add_kind = .replace
        mods.choose_position(.all)
        let editing: MOD_PAGE = mods.page
        let apply_label: String = mods.apply_label
        scenario.editor.part = .ext
        scenario.editor.value = [VALUE_PIECE(text: "jpeg")]
        let applied: Bool = try mods.apply()

        #expect(insert == KIND_CARD(kind: .insert, name: "넣기",
            description: "원하는 위치에 텍스트, 번호, 날짜 넣기", example_before: "IMG_0001.JPG",
            example_after: "2024_IMG_0001.JPG"))
        #expect(badge == "신규")
        #expect(editing == .edit)
        #expect(apply_label == "추가")
        #expect(applied)
        #expect(mods.page == .list)
        #expect(mods.cards.first?.summary == "확장자를 \"jpeg\"로 바꾸기")
        #expect(try scenario.first_new_name() == "a.jpeg")
        #expect(scenario.messages == ["수정을 추가했습니다."])
    }

    @Test func goes_back_to_the_previous_page() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        mods.open_add()
        mods.add_kind = .delete
        mods.choose_position(.first_n)
        let new_label: String = mods.back_label
        mods.go_back()
        let after_new: MOD_PAGE = mods.page
        let kept_kind: MOD_KIND = mods.add_kind
        let add_label: String = mods.add_back_label
        mods.go_back()
        let after_add: MOD_PAGE = mods.page
        mods.open_card(mods.cards[0])
        let edit_label: String = mods.back_label
        mods.go_back()

        #expect(new_label == "수정 추가")
        #expect(after_new == .add)
        #expect(kept_kind == .delete)
        #expect(add_label == "수정 목록")
        #expect(after_add == .list)
        #expect(edit_label == "수정 목록")
        #expect(mods.page == .list)
        #expect(scenario.messages.isEmpty)
    }

    @Test func switches_kinds_of_new_and_existing_mods() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        mods.open_add()
        mods.choose_position(.end)
        mods.switch_kind(.delete)
        let new_page: MOD_PAGE = mods.page
        let new_active: Bool = scenario.editor.is_active
        let new_back: String = mods.add_back_label
        mods.go_back()
        let after_new_back: MOD_PAGE = mods.page

        let card: MOD_CARD = mods.cards[0]
        mods.open_card(card)
        let badge: String = mods.badge_text
        let apply_label: String = mods.apply_label
        mods.edit_kind = .insert
        let same_kind: MOD_PAGE = mods.page
        mods.edit_kind = .replace
        let edit_back: String = mods.add_back_label
        let kept_badge: String = mods.badge_text
        mods.go_back()
        let restored: MOD_KIND = scenario.editor.kind
        mods.switch_kind(.replace)
        mods.choose_position(.all)
        let editing: UInt64 = scenario.editor.editing_id
        scenario.editor.value = [VALUE_PIECE(text: "b")]
        let applied: Bool = try mods.apply()

        #expect(new_page == .add)
        #expect(!new_active)
        #expect(new_back == "수정 목록")
        #expect(after_new_back == .list)
        #expect(badge == "편집 중")
        #expect(apply_label == "적용")
        #expect(same_kind == .edit)
        #expect(edit_back == "편집")
        #expect(kept_badge == "편집 중")
        #expect(restored == .insert)
        #expect(editing == card.id)
        #expect(applied)
        #expect(mods.cards.count == 1)
        #expect(mods.cards.first?.id == card.id)
        #expect(mods.cards.first?.mod.kind == .replace)
        #expect(try scenario.first_new_name() == "b.txt")
        #expect(scenario.messages == ["수정을 고쳤습니다."])
    }

    @Test func cancels_a_switched_kind_with_the_original_kept() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        mods.open_card(mods.cards[0])
        mods.switch_kind(.delete)
        mods.choose_position(.all)
        mods.cancel()

        #expect(mods.cards.first?.mod.kind == .insert)
        #expect(try scenario.first_new_name() == "new_a.txt")
        #expect(scenario.messages == ["편집을 취소했습니다."])
    }

    @Test func turns_an_edited_direct_edit_into_a_plain_mod() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        try scenario.engine.mod_direct_edit(try scenario.items.id("a.txt"), new_name: "b.txt")
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        mods.open_card(mods.cards[0])
        let summary: String = scenario.editor.summary
        scenario.editor.value = [VALUE_PIECE(text: "c.txt")]
        _ = try mods.apply()

        #expect(summary == "이름과 확장자를 \"b.txt\"로 바꾸기 (1개)")
        #expect(mods.cards.first?.direct == false)
        #expect(mods.cards.first?.summary == "이름과 확장자를 \"c.txt\"로 바꾸기 (1개)")
        #expect(try scenario.first_new_name() == "c.txt")
    }

    @Test func cancels_with_messages() throws
    {
        let scenario = try MOD_PANEL_SCENARIO("a.txt")
        defer {
            scenario.close()
        }
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        try scenario.load()
        let mods: MOD_LIST_VIEW_MODEL = scenario.mods

        mods.open_add()
        mods.add_kind = .delete
        mods.choose_position(.all)
        mods.cancel()
        mods.open_card(mods.cards[0])
        mods.cancel()
        mods.open_card(mods.cards[0])
        scenario.editor.value = [VALUE_PIECE(text: "x_")]
        mods.cancel()

        #expect(scenario.messages == [
            "새 수정을 취소했습니다.",
            "편집을 취소했습니다.",
        ])
        #expect(try scenario.first_new_name() == "new_a.txt")
        #expect(mods.page == .list)
    }
}
