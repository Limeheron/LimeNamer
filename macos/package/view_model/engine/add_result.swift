public struct ADD_RESULT: Equatable, Sendable
{
    public let added: UInt32
    public let skipped: UInt32
    public let failed: UInt32

    public init(added: UInt32, skipped: UInt32, failed: UInt32)
    {
        self.added = added
        self.skipped = skipped
        self.failed = failed
    }
}
