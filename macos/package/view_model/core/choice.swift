public struct CHOICE<T: Equatable & Sendable>: Equatable, Sendable
{
    public let value: T
    public let label: String

    public init(value: T, label: String)
    {
        self.value = value
        self.label = label
    }
}
