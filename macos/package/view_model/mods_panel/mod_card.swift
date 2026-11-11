public struct MOD_CARD: Equatable, Identifiable, Sendable
{
    public let id: UInt64
    public let enabled: Bool
    public let direct: Bool
    public let summary: String
    public let mod: MOD

    public init(id: UInt64, enabled: Bool, direct: Bool, summary: String, mod: MOD)
    {
        self.id = id
        self.enabled = enabled
        self.direct = direct
        self.summary = summary
        self.mod = mod
    }
}
