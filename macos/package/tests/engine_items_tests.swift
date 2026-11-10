import Testing

import limenamer_view_model

struct ENGINE_ITEMS_TESTS
{
    @Test func adds_items_and_reads_preview() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let a: String = try temp.file("a.txt", "abc")
        let b: String = try temp.file("b.txt")
        let photos: String = try temp.folder("photos")
        let engine = try ENGINE()

        let result: ADD_RESULT = try engine.items_add([a, b, photos], mode: .as_items)
        let preview: PREVIEW = try engine.preview()

        #expect(result == ADD_RESULT(added: 3, skipped: 0, failed: 0))
        try #require(preview.rows.count == 3)
        let first: ROW = preview.rows[0]
        #expect(first.folder == temp.path)
        #expect(first.name == "a.txt")
        #expect(first.new_name == "a.txt")
        #expect(first.size == 3)
        #expect(first.modified_ms > 0)
        #expect(first.status == .unchanged)
        #expect(!first.changed)
        #expect(preview.rows[1].folder == first.folder)
        #expect(preview.rows[2].is_dir)
        #expect(preview.changed == 0)
    }

    @Test func counts_invalid_paths_as_failed() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let a: String = try temp.file("a.txt")
        let broken: String = temp.path + "/b\u{0}.txt"
        let engine = try ENGINE()

        let first: ADD_RESULT = try engine.items_add([a, broken], mode: .as_items)
        let second: ADD_RESULT = try engine.items_add([a], mode: .as_items)

        #expect(first == ADD_RESULT(added: 1, skipped: 0, failed: 1))
        #expect(second == ADD_RESULT(added: 0, skipped: 1, failed: 0))
    }

    @Test func adds_folder_contents() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        _ = try temp.file("a.txt")
        _ = try temp.file("b.txt")
        let engine = try ENGINE()

        let result: ADD_RESULT = try engine.items_add([temp.path], mode: .folder_contents)

        #expect(result == ADD_RESULT(added: 2, skipped: 0, failed: 0))
    }

    @Test func sorts_and_moves_items() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt"), try temp.file("b.txt")],
            mode: .as_items)

        try engine.items_sort(.name_natural, descending: true)
        let sorted: PREVIEW = try engine.preview()
        let b_id: UInt64 = sorted.rows[0].item_id
        let a_id: UInt64 = sorted.rows[1].item_id

        #expect(sorted.rows[0].name == "b.txt")
        #expect(try engine.items_sort_state() == (.name_natural, true))
        #expect(try engine.items_move([a_id], target_id: b_id, after: false, dry_run: true))
        #expect(try engine.preview().rows[0].name == "b.txt")
        #expect(try engine.items_move([a_id], target_id: b_id, after: false, dry_run: false))
        #expect(try engine.preview().rows[0].name == "a.txt")
        #expect(try engine.items_sort_state().key == .custom)
    }

    @Test func removes_and_clears_items() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt"), try temp.file("b.txt")],
            mode: .as_items)
        let first: UInt64 = try engine.preview().rows[0].item_id

        try engine.items_remove([first])
        #expect(throws: ENGINE_ERROR(result: .not_found)) {
            try engine.items_remove([first])
        }
        let remaining: Int = try engine.preview().rows.count
        try engine.items_clear()

        #expect(remaining == 1)
        #expect(try engine.preview().rows.isEmpty)
    }

    @Test func refreshes_folders() throws
    {
        let temp = try TEMP_FOLDER()
        defer {
            temp.remove()
        }
        let engine = try ENGINE()
        _ = try engine.items_add([try temp.file("a.txt")], mode: .as_items)
        _ = try engine.mod_add(TEST_MODS.INSERT_NEW)
        _ = try temp.file("new_a.txt")

        let before: STATUS = try engine.preview().rows[0].status
        try engine.folders_refresh()

        #expect(before == .ok)
        #expect(try engine.preview().rows[0].status == .conflict)
    }
}
