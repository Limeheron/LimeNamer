import Testing

import limenamer_view_model

struct ENGINE_MODS_TESTS
{
    @Test func applies_mods_to_preview() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt")], mode: .as_items)

        let id: UInt64 = try engine.mod_add(TEST_MODS.INSERT_NEW)
        let preview: PREVIEW = try engine.preview()

        let row: ROW = preview.rows[0]
        #expect(row.new_name == "new_a.txt")
        #expect(row.status == .ok)
        #expect(row.changed)
        #expect(row.prefix_len == 0)
        #expect(row.suffix_len == 5)
        #expect(preview.changed == 1)
        #expect(preview.runnable == 1)
        let mods: String = try engine.mods_get()
        #expect(mods.contains("\"id\":\(id)"), "\(mods)")
    }

    @Test func updates_toggles_and_removes_mods() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt")], mode: .as_items)
        let first: UInt64 = try engine.mod_add(TEST_MODS.INSERT_NEW)
        let second: UInt64 = try engine.mod_add(TEST_MODS.INSERT_OLD)

        let both: String = try engine.preview().rows[0].new_name
        let moved: Bool = try engine.mod_move(second, target_id: first, after: false,
            dry_run: false)
        let reordered: String = try engine.preview().rows[0].new_name
        try engine.mod_update(first, json: TEST_MODS.INSERT_OLD)
        let updated: String = try engine.preview().rows[0].new_name
        try engine.mods_set_all_enabled(false)
        let disabled: String = try engine.preview().rows[0].new_name
        try engine.mod_set_enabled(first, enabled: true)
        let enabled: String = try engine.preview().rows[0].new_name
        try engine.mod_remove(first)

        #expect(both == "old_new_a.txt")
        #expect(moved)
        #expect(reordered == "new_old_a.txt")
        #expect(updated == "old_old_a.txt")
        #expect(disabled == "a.txt")
        #expect(enabled == "old_a.txt")
        #expect(try engine.preview().rows[0].new_name == "a.txt")
    }

    @Test func reports_check_problems() throws
    {
        let engine = try ENGINE()

        let problems: [PROBLEM] = try engine.mod_check(
            #"{"kind":"replace","position":{"type":"text","text":""},"value":[{"text":"x"}]}"#)

        #expect(problems.contains(PROBLEM(field: .find_text, reason: .required)))
    }

    @Test func finds_examples() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt")], mode: .as_items)
        let item_id: UInt64 = try engine.preview().rows[0].item_id

        let example: EXAMPLE = try engine.mod_example(TEST_MODS.INSERT_NEW, before_mod_id: 0)

        #expect(example == EXAMPLE(item_id: item_id, index: 1, before: "a.txt",
            after: "new_a.txt", prefix_len: 0, suffix_len: 5, errors: .none, state: .changed))
    }

    @Test func edits_names_directly() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt")], mode: .as_items)
        let item_id: UInt64 = try engine.preview().rows[0].item_id

        try engine.mod_direct_edit(item_id, new_name: "b.txt")
        let row: ROW = try engine.preview().rows[0]

        #expect(row.new_name == "b.txt")
        #expect(row.direct)
        #expect(throws: ENGINE_ERROR(result: .invalid_argument)) {
            try engine.mod_direct_edit(item_id, new_name: "b\u{0}.txt")
        }
    }

    @Test func reports_conflicts_and_issues() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        _ = try temp.file("new_a.txt")
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt"), try temp.file("b.txt")],
            mode: .as_items)
        let b_id: UInt64 = try engine.preview().rows[1].item_id

        _ = try engine.mod_add(TEST_MODS.INSERT_NEW)
        try engine.mod_direct_edit(b_id, new_name: "b:.txt")
        let preview: PREVIEW = try engine.preview()

        #expect(preview.rows[0].status == .conflict)
        #expect(preview.rows[0].conflict == .existing_item)
        #expect(preview.rows[1].status == .error)
        #expect(preview.rows[1].errors == .mac_forbidden_char)
        #expect(preview.rows[1].warnings.contains(.win_forbidden_char))
        #expect(preview.conflict == 1)
        #expect(preview.error == 1)
    }

    @Test func rejects_invalid_json() throws
    {
        let engine = try ENGINE()

        #expect(throws: ENGINE_ERROR(result: .invalid_json)) {
            try engine.mod_add("{")
        }
    }

    @Test func tracks_history() throws
    {
        let engine = try ENGINE()

        #expect(throws: ENGINE_ERROR(result: .not_found)) {
            try engine.undo()
        }
        _ = try engine.mod_add(TEST_MODS.INSERT_NEW)
        let added: (can_undo: Bool, can_redo: Bool) = try engine.history_state()
        try engine.undo()
        let undone: String = try engine.mods_get()
        let after_undo: (can_undo: Bool, can_redo: Bool) = try engine.history_state()
        try engine.redo()

        #expect(added == (true, false))
        #expect(undone == "[]")
        #expect(after_undo == (false, true))
        #expect(try engine.mods_get() != "[]")
    }

    @Test func reads_and_sets_options() throws
    {
        let engine = try ENGINE()

        try engine.options_set(#"{"duplicate":{"enabled":true}}"#)
        let options: String = try engine.options_get()

        #expect(options.contains("\"enabled\":true"), "\(options)")
    }
}
