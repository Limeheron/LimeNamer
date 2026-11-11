public struct KIND_CARD: Equatable, Sendable
{
    public let kind: MOD_KIND
    public let name: String
    public let description: String
    public let example_before: String
    public let example_after: String

    public init(kind: MOD_KIND, name: String, description: String, example_before: String,
        example_after: String)
    {
        self.kind = kind
        self.name = name
        self.description = description
        self.example_before = example_before
        self.example_after = example_after
    }
}
