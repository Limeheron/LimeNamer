public struct PROBLEM: Hashable, Sendable
{
    public let field: MOD_FIELD
    public let reason: PROBLEM_REASON

    public init(field: MOD_FIELD, reason: PROBLEM_REASON)
    {
        self.field = field
        self.reason = reason
    }
}
