import limenamer_engine

public final class ENGINE: @unchecked Sendable
{
    private typealias RUN_CALL = (LN_PROGRESS_FN?, UnsafeMutableRawPointer?,
        UnsafeMutablePointer<UnsafeMutablePointer<LN_REPORT>?>) -> LN_RESULT

    private let session: OpaquePointer

    public init() throws(ENGINE_ERROR)
    {
        var session: OpaquePointer? = nil
        let result: LN_RESULT = ln_session_create(&session)
        guard result == RESULT.ok.rawValue, let session else {
            ln_session_destroy(session)
            throw ENGINE.error(result)
        }
        self.session = session
    }

    deinit
    {
        ln_session_destroy(session)
    }

    public static func version() -> String
    {
        return NATIVE_READER.read_string(ln_version())
    }

    public func items_add(_ paths: [String], mode: ADD_MODE) throws(ENGINE_ERROR) -> ADD_RESULT
    {
        var buffer: [CChar] = []
        var offsets: [Int] = []
        var invalid: UInt32 = 0
        for path in paths {
            if UTF8_INPUT.is_valid(path) {
                offsets.append(buffer.count)
                buffer.append(contentsOf: path.utf8.map { CChar(bitPattern: $0) })
                buffer.append(0)
            } else {
                invalid += 1
            }
        }
        var added = LN_ADD_RESULT()
        let result: LN_RESULT = buffer.withUnsafeBufferPointer { start in
            let pointers: [UnsafePointer<CChar>?] = offsets.map { start.baseAddress! + $0 }
            return ln_items_add(session, pointers, pointers.count, mode.rawValue, &added)
        }
        try ENGINE.check(result)
        return ADD_RESULT(added: added.added, skipped: added.skipped,
            failed: added.failed + invalid)
    }

    public func items_remove(_ ids: [UInt64]) throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_items_remove(session, ids, ids.count))
    }

    public func items_clear() throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_items_clear(session))
    }

    public func items_move(_ ids: [UInt64], target_id: UInt64, after: Bool, dry_run: Bool)
        throws(ENGINE_ERROR) -> Bool
    {
        var changed: UInt8 = 0
        try ENGINE.check(ln_items_move(session, ids, ids.count, target_id, ENGINE.to_byte(after),
            ENGINE.to_byte(dry_run), &changed))
        return changed != 0
    }

    public func items_sort(_ key: SORT_KEY, descending: Bool) throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_items_sort(session, key.rawValue, ENGINE.to_byte(descending)))
    }

    public func items_sort_state() throws(ENGINE_ERROR) -> (key: SORT_KEY, descending: Bool)
    {
        var key: Int32 = 0
        var descending: UInt8 = 0
        try ENGINE.check(ln_items_sort_state(session, &key, &descending))
        guard let sort_key = SORT_KEY(rawValue: key) else {
            throw ENGINE_ERROR(result: .internal_error)
        }
        return (sort_key, descending != 0)
    }

    public func folders_refresh() throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_folders_refresh(session))
    }

    public func mods_get() throws(ENGINE_ERROR) -> String
    {
        var json: UnsafeMutablePointer<CChar>? = nil
        try ENGINE.check(ln_mods_get(session, &json))
        return try NATIVE_READER.take_string(json)
    }

    public func mod_add(_ json: String) throws(ENGINE_ERROR) -> UInt64
    {
        var id: UInt64 = 0
        try ENGINE.check(try UTF8_INPUT.with_c_string(json) { ln_mod_add(session, $0, &id) })
        return id
    }

    public func mod_update(_ id: UInt64, json: String) throws(ENGINE_ERROR)
    {
        try ENGINE.check(try UTF8_INPUT.with_c_string(json) { ln_mod_update(session, id, $0) })
    }

    public func mod_remove(_ id: UInt64) throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_mod_remove(session, id))
    }

    public func mod_move(_ id: UInt64, target_id: UInt64, after: Bool, dry_run: Bool)
        throws(ENGINE_ERROR) -> Bool
    {
        var changed: UInt8 = 0
        try ENGINE.check(ln_mod_move(session, id, target_id, ENGINE.to_byte(after),
            ENGINE.to_byte(dry_run), &changed))
        return changed != 0
    }

    public func mod_set_enabled(_ id: UInt64, enabled: Bool) throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_mod_set_enabled(session, id, ENGINE.to_byte(enabled)))
    }

    public func mods_set_all_enabled(_ enabled: Bool) throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_mods_set_all_enabled(session, ENGINE.to_byte(enabled)))
    }

    public func mod_direct_edit(_ item_id: UInt64, new_name: String) throws(ENGINE_ERROR)
    {
        try ENGINE.check(try UTF8_INPUT.with_c_string(new_name) {
            ln_mod_direct_edit(session, item_id, $0)
        })
    }

    public func mod_check(_ json: String) throws(ENGINE_ERROR) -> [PROBLEM]
    {
        var problems: UnsafeMutablePointer<LN_PROBLEMS>? = nil
        try ENGINE.check(try UTF8_INPUT.with_c_string(json) {
            ln_mod_check(session, $0, &problems)
        })
        return try NATIVE_READER.take_problems(problems)
    }

    public func mod_example(_ json: String, before_mod_id: UInt64) throws(ENGINE_ERROR)
        -> EXAMPLE
    {
        var example: UnsafeMutablePointer<LN_EXAMPLE>? = nil
        try ENGINE.check(try UTF8_INPUT.with_c_string(json) {
            ln_mod_example(session, $0, before_mod_id, &example)
        })
        return try NATIVE_READER.take_example(example)
    }

    public func options_get() throws(ENGINE_ERROR) -> String
    {
        var json: UnsafeMutablePointer<CChar>? = nil
        try ENGINE.check(ln_options_get(session, &json))
        return try NATIVE_READER.take_string(json)
    }

    public func options_set(_ json: String) throws(ENGINE_ERROR)
    {
        try ENGINE.check(try UTF8_INPUT.with_c_string(json) { ln_options_set(session, $0) })
    }

    public func preview() throws(ENGINE_ERROR) -> PREVIEW
    {
        var preview: UnsafeMutablePointer<LN_PREVIEW>? = nil
        try ENGINE.check(ln_preview(session, &preview))
        return try NATIVE_READER.take_preview(preview)
    }

    public func undo() throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_undo(session))
    }

    public func redo() throws(ENGINE_ERROR)
    {
        try ENGINE.check(ln_redo(session))
    }

    public func history_state() throws(ENGINE_ERROR) -> (can_undo: Bool, can_redo: Bool)
    {
        var can_undo: UInt8 = 0
        var can_redo: UInt8 = 0
        try ENGINE.check(ln_history_state(session, &can_undo, &can_redo))
        return (can_undo != 0, can_redo != 0)
    }

    public func rename(progress: PROGRESS?) throws(ENGINE_ERROR) -> REPORT
    {
        return try ENGINE.run(progress) { ln_rename(session, $0, $1, $2) }
    }

    public func rename_resolve(_ choice: RESOLVE_CHOICE, progress: PROGRESS?)
        throws(ENGINE_ERROR) -> REPORT
    {
        return try ENGINE.run(progress) {
            ln_rename_resolve(session, choice.rawValue, $0, $1, $2)
        }
    }

    public func rename_undo_check() throws(ENGINE_ERROR) -> REPORT
    {
        var report: UnsafeMutablePointer<LN_REPORT>? = nil
        try ENGINE.check(ln_rename_undo_check(session, &report))
        return try NATIVE_READER.take_report(report)
    }

    public func rename_undo(progress: PROGRESS?) throws(ENGINE_ERROR) -> REPORT
    {
        return try ENGINE.run(progress) { ln_rename_undo(session, $0, $1, $2) }
    }

    public func rename_undo_state() throws(ENGINE_ERROR) -> (available: Bool, count: UInt32)
    {
        var available: UInt8 = 0
        var count: UInt32 = 0
        try ENGINE.check(ln_rename_undo_state(session, &available, &count))
        return (available != 0, count)
    }

    public static func index_list_parse(_ text: String, item_count: UInt32)
        throws(ENGINE_ERROR) -> INDEX_LIST
    {
        var list: UnsafeMutablePointer<LN_INDEX_LIST>? = nil
        try check(try UTF8_INPUT.with_c_string(text) {
            ln_index_list_parse($0, item_count, &list)
        })
        return try NATIVE_READER.take_index_list(list)
    }

    public static func index_list_format(_ indices: [UInt32]) throws(ENGINE_ERROR) -> String
    {
        var text: UnsafeMutablePointer<CChar>? = nil
        try check(ln_index_list_format(indices, indices.count, &text))
        return try NATIVE_READER.take_string(text)
    }

    private static func run(_ progress: PROGRESS?, _ call: RUN_CALL) throws(ENGINE_ERROR)
        -> REPORT
    {
        var report: UnsafeMutablePointer<LN_REPORT>? = nil
        let result: LN_RESULT = withExtendedLifetime(progress) {
            guard let progress else {
                return call(nil, nil, &report)
            }
            return call(on_progress, Unmanaged.passUnretained(progress).toOpaque(), &report)
        }
        try check(result)
        return try NATIVE_READER.take_report(report)
    }

    private static func check(_ result: LN_RESULT) throws(ENGINE_ERROR)
    {
        if result != RESULT.ok.rawValue {
            throw error(result)
        }
    }

    private static func error(_ result: LN_RESULT) -> ENGINE_ERROR
    {
        return ENGINE_ERROR(result: RESULT(rawValue: result) ?? .internal_error)
    }

    private static func to_byte(_ value: Bool) -> UInt8
    {
        return value ? 1 : 0
    }
}

private func on_progress(_ user: UnsafeMutableRawPointer?, _ done: UInt32, _ total: UInt32)
{
    guard let user else {
        return
    }
    Unmanaged<PROGRESS>.fromOpaque(user).takeUnretainedValue().set(done, total)
}
