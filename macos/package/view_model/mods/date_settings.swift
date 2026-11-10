public struct DATE_SETTINGS: Codable, Equatable, Sendable
{
    public var format: String

    public init(format: String = "yyyyMMdd")
    {
        self.format = format
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        format = try container.decodeIfPresent(String.self, forKey: .format) ?? "yyyyMMdd"
    }
}
