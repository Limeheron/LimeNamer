public struct REPORT: Equatable, Sendable
{
    public let problems: [REPORT_ITEM]
    public let total: UInt32
    public let succeeded: UInt32
    public let skipped: UInt32
    public let failed: UInt32
    public let needs_choice: Bool

    public init(problems: [REPORT_ITEM], total: UInt32, succeeded: UInt32, skipped: UInt32,
        failed: UInt32, needs_choice: Bool)
    {
        self.problems = problems
        self.total = total
        self.succeeded = succeeded
        self.skipped = skipped
        self.failed = failed
        self.needs_choice = needs_choice
    }
}
