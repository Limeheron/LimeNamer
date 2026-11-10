public struct EXAMPLE: Equatable, Sendable
{
    public let item_id: UInt64
    public let index: UInt32
    public let before: String
    public let after: String
    public let prefix_len: UInt32
    public let suffix_len: UInt32
    public let errors: NAME_ISSUE
    public let state: EXAMPLE_STATE

    public init(item_id: UInt64, index: UInt32, before: String, after: String,
        prefix_len: UInt32, suffix_len: UInt32, errors: NAME_ISSUE, state: EXAMPLE_STATE)
    {
        self.item_id = item_id
        self.index = index
        self.before = before
        self.after = after
        self.prefix_len = prefix_len
        self.suffix_len = suffix_len
        self.errors = errors
        self.state = state
    }
}
