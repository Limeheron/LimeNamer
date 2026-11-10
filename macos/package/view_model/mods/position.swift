public struct POSITION: Codable, Equatable, Sendable
{
    public var type: POSITION_TYPE
    public var n: UInt32?
    public var count: UInt32?
    public var text: String?
    public var case_sensitive: Bool?
    public var occurrence: OCCURRENCE?
    public var occurrence_n: UInt32?
    public var from_beginning: Bool?
    public var start_text: String?
    public var to_end: Bool?
    public var end_text: String?
    public var include_delimiters: Bool?

    public init(type: POSITION_TYPE = .start, n: UInt32? = nil, count: UInt32? = nil,
        text: String? = nil, case_sensitive: Bool? = nil, occurrence: OCCURRENCE? = nil,
        occurrence_n: UInt32? = nil, from_beginning: Bool? = nil, start_text: String? = nil,
        to_end: Bool? = nil, end_text: String? = nil, include_delimiters: Bool? = nil)
    {
        self.type = type
        self.n = n
        self.count = count
        self.text = text
        self.case_sensitive = case_sensitive
        self.occurrence = occurrence
        self.occurrence_n = occurrence_n
        self.from_beginning = from_beginning
        self.start_text = start_text
        self.to_end = to_end
        self.end_text = end_text
        self.include_delimiters = include_delimiters
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        type = try container.decodeIfPresent(POSITION_TYPE.self, forKey: .type) ?? .start
        n = try container.decodeIfPresent(UInt32.self, forKey: .n)
        count = try container.decodeIfPresent(UInt32.self, forKey: .count)
        text = try container.decodeIfPresent(String.self, forKey: .text)
        case_sensitive = try container.decodeIfPresent(Bool.self, forKey: .case_sensitive)
        occurrence = try container.decodeIfPresent(OCCURRENCE.self, forKey: .occurrence)
        occurrence_n = try container.decodeIfPresent(UInt32.self, forKey: .occurrence_n)
        from_beginning = try container.decodeIfPresent(Bool.self, forKey: .from_beginning)
        start_text = try container.decodeIfPresent(String.self, forKey: .start_text)
        to_end = try container.decodeIfPresent(Bool.self, forKey: .to_end)
        end_text = try container.decodeIfPresent(String.self, forKey: .end_text)
        include_delimiters = try container.decodeIfPresent(Bool.self, forKey: .include_delimiters)
    }
}
