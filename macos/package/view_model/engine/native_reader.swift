import limenamer_engine

enum NATIVE_READER
{
    static func read_string(_ text: UnsafePointer<CChar>) -> String
    {
        return String(cString: text)
    }

    static func take_string(_ text: UnsafeMutablePointer<CChar>?) throws(ENGINE_ERROR) -> String
    {
        defer {
            ln_string_free(text)
        }
        return read_string(try require(text))
    }

    static func take_preview(_ preview: UnsafeMutablePointer<LN_PREVIEW>?)
        throws(ENGINE_ERROR) -> PREVIEW
    {
        defer {
            ln_preview_free(preview)
        }
        let source: LN_PREVIEW = try require(preview).pointee
        var rows: [ROW] = []
        var folders: [UnsafePointer<CChar>: String] = [:]
        rows.reserveCapacity(source.count)
        for row in UnsafeBufferPointer(start: source.rows, count: source.count) {
            let folder: String
            if let cached = folders[row.folder] {
                folder = cached
            } else {
                folder = read_string(row.folder)
                folders[row.folder] = folder
            }
            rows.append(ROW(item_id: row.item_id, folder: folder, name: read_string(row.name),
                new_name: read_string(row.new_name), size: row.size,
                modified_ms: row.modified_ms, created_ms: row.created_ms,
                prefix_len: row.prefix_len, suffix_len: row.suffix_len,
                errors: NAME_ISSUE(rawValue: row.errors),
                warnings: NAME_ISSUE(rawValue: row.warnings), status: try value(row.status),
                conflict: try value(row.conflict), conflict_index: row.conflict_index,
                is_dir: row.is_dir != 0, changed: row.changed != 0, adjusted: row.adjusted != 0,
                direct: row.direct != 0))
        }
        return PREVIEW(rows: rows, changed: source.changed, runnable: source.runnable,
            warning: source.warning, error: source.error, conflict: source.conflict)
    }

    static func take_example(_ example: UnsafeMutablePointer<LN_EXAMPLE>?)
        throws(ENGINE_ERROR) -> EXAMPLE
    {
        defer {
            ln_example_free(example)
        }
        let source: LN_EXAMPLE = try require(example).pointee
        return EXAMPLE(item_id: source.item_id, index: source.index,
            before: read_string(source.before), after: read_string(source.after),
            prefix_len: source.prefix_len, suffix_len: source.suffix_len,
            errors: NAME_ISSUE(rawValue: source.errors), state: try value(source.state))
    }

    static func take_report(_ report: UnsafeMutablePointer<LN_REPORT>?)
        throws(ENGINE_ERROR) -> REPORT
    {
        defer {
            ln_report_free(report)
        }
        let source: LN_REPORT = try require(report).pointee
        var problems: [REPORT_ITEM] = []
        problems.reserveCapacity(source.problem_count)
        for item in UnsafeBufferPointer(start: source.problems, count: source.problem_count) {
            problems.append(REPORT_ITEM(item_id: item.item_id,
                from_path: read_string(item.from_path), to_path: read_string(item.to_path),
                current_path: read_string(item.current_path), stage: try value(item.stage),
                failure: try value(item.failure), os_error: item.os_error))
        }
        return REPORT(problems: problems, total: source.total, succeeded: source.succeeded,
            skipped: source.skipped, failed: source.failed, needs_choice: source.needs_choice != 0)
    }

    static func take_problems(_ problems: UnsafeMutablePointer<LN_PROBLEMS>?)
        throws(ENGINE_ERROR) -> [PROBLEM]
    {
        defer {
            ln_problems_free(problems)
        }
        let source: LN_PROBLEMS = try require(problems).pointee
        var items: [PROBLEM] = []
        items.reserveCapacity(source.count)
        for item in UnsafeBufferPointer(start: source.items, count: source.count) {
            items.append(PROBLEM(field: try value(item.field), reason: try value(item.reason)))
        }
        return items
    }

    static func take_index_list(_ list: UnsafeMutablePointer<LN_INDEX_LIST>?)
        throws(ENGINE_ERROR) -> INDEX_LIST
    {
        defer {
            ln_index_list_free(list)
        }
        let source: LN_INDEX_LIST = try require(list).pointee
        return INDEX_LIST(indices: Array(UnsafeBufferPointer(start: source.indices,
            count: source.count)), error: try value(source.error))
    }

    private static func require<T>(_ pointer: T?) throws(ENGINE_ERROR) -> T
    {
        guard let pointer else {
            throw ENGINE_ERROR(result: .internal_error)
        }
        return pointer
    }

    private static func value<T: RawRepresentable>(_ raw: T.RawValue) throws(ENGINE_ERROR) -> T
    {
        guard let value = T(rawValue: raw) else {
            throw ENGINE_ERROR(result: .internal_error)
        }
        return value
    }
}
