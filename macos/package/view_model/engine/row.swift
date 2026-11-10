public struct ROW: Equatable, Sendable
{
    public let item_id: UInt64
    public let folder: String
    public let name: String
    public let new_name: String
    public let size: Int64
    public let modified_ms: Int64
    public let created_ms: Int64
    public let prefix_len: UInt32
    public let suffix_len: UInt32
    public let errors: NAME_ISSUE
    public let warnings: NAME_ISSUE
    public let status: STATUS
    public let conflict: CONFLICT
    public let conflict_index: UInt32
    public let is_dir: Bool
    public let changed: Bool
    public let adjusted: Bool
    public let direct: Bool

    public init(item_id: UInt64, folder: String, name: String, new_name: String, size: Int64,
        modified_ms: Int64, created_ms: Int64, prefix_len: UInt32, suffix_len: UInt32,
        errors: NAME_ISSUE, warnings: NAME_ISSUE, status: STATUS, conflict: CONFLICT,
        conflict_index: UInt32, is_dir: Bool, changed: Bool, adjusted: Bool, direct: Bool)
    {
        self.item_id = item_id
        self.folder = folder
        self.name = name
        self.new_name = new_name
        self.size = size
        self.modified_ms = modified_ms
        self.created_ms = created_ms
        self.prefix_len = prefix_len
        self.suffix_len = suffix_len
        self.errors = errors
        self.warnings = warnings
        self.status = status
        self.conflict = conflict
        self.conflict_index = conflict_index
        self.is_dir = is_dir
        self.changed = changed
        self.adjusted = adjusted
        self.direct = direct
    }
}
