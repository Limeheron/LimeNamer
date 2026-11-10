public struct DUPLICATE_OPTIONS: Codable, Equatable, Sendable
{
    public var enabled: Bool
    public var format: DUPLICATE_FORMAT
    public var custom_text: String?
    public var mode: DUPLICATE_MODE

    public init(enabled: Bool = false, format: DUPLICATE_FORMAT = .paren,
        custom_text: String? = nil, mode: DUPLICATE_MODE = .keep_first)
    {
        self.enabled = enabled
        self.format = format
        self.custom_text = custom_text
        self.mode = mode
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        enabled = try container.decodeIfPresent(Bool.self, forKey: .enabled) ?? false
        format = try container.decodeIfPresent(DUPLICATE_FORMAT.self, forKey: .format) ?? .paren
        custom_text = try container.decodeIfPresent(String.self, forKey: .custom_text)
        mode = try container.decodeIfPresent(DUPLICATE_MODE.self, forKey: .mode) ?? .keep_first
    }
}
