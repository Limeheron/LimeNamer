public struct MOD: Codable, Equatable, Sendable
{
    public var id: UInt64?
    public var enabled: Bool?
    public var kind: MOD_KIND
    public var part: PART
    public var position: POSITION
    public var value: [VALUE_PIECE]?
    public var tokens: TOKEN_SETTINGS?
    public var format: FORMAT_SETTINGS?
    public var scope: SCOPE
    public var targets: [UInt64]?
    public var direct: Bool

    public init(id: UInt64? = nil, enabled: Bool? = nil, kind: MOD_KIND = .insert,
        part: PART = .name, position: POSITION = POSITION(), value: [VALUE_PIECE]? = nil,
        tokens: TOKEN_SETTINGS? = nil, format: FORMAT_SETTINGS? = nil, scope: SCOPE = .all,
        targets: [UInt64]? = nil, direct: Bool = false)
    {
        self.id = id
        self.enabled = enabled
        self.kind = kind
        self.part = part
        self.position = position
        self.value = value
        self.tokens = tokens
        self.format = format
        self.scope = scope
        self.targets = targets
        self.direct = direct
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        id = try container.decodeIfPresent(UInt64.self, forKey: .id)
        enabled = try container.decodeIfPresent(Bool.self, forKey: .enabled)
        kind = try container.decodeIfPresent(MOD_KIND.self, forKey: .kind) ?? .insert
        part = try container.decodeIfPresent(PART.self, forKey: .part) ?? .name
        position = try container.decodeIfPresent(POSITION.self, forKey: .position) ?? POSITION()
        value = try container.decodeIfPresent([VALUE_PIECE].self, forKey: .value)
        tokens = try container.decodeIfPresent(TOKEN_SETTINGS.self, forKey: .tokens)
        format = try container.decodeIfPresent(FORMAT_SETTINGS.self, forKey: .format)
        scope = try container.decodeIfPresent(SCOPE.self, forKey: .scope) ?? .all
        targets = try container.decodeIfPresent([UInt64].self, forKey: .targets)
        direct = try container.decodeIfPresent(Bool.self, forKey: .direct) ?? false
    }
}
