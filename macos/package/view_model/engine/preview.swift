public struct PREVIEW: Equatable, Sendable
{
    public let rows: [ROW]
    public let changed: UInt32
    public let runnable: UInt32
    public let warning: UInt32
    public let error: UInt32
    public let conflict: UInt32

    public init(rows: [ROW], changed: UInt32, runnable: UInt32, warning: UInt32, error: UInt32,
        conflict: UInt32)
    {
        self.rows = rows
        self.changed = changed
        self.runnable = runnable
        self.warning = warning
        self.error = error
        self.conflict = conflict
    }
}
