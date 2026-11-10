public struct NAME_ISSUE: OptionSet, Hashable, Sendable
{
    public static let none: NAME_ISSUE = []
    public static let empty = NAME_ISSUE(rawValue: 1 << 0)
    public static let dot_name = NAME_ISSUE(rawValue: 1 << 1)
    public static let win_forbidden_char = NAME_ISSUE(rawValue: 1 << 2)
    public static let win_reserved = NAME_ISSUE(rawValue: 1 << 3)
    public static let win_trailing = NAME_ISSUE(rawValue: 1 << 4)
    public static let win_name_too_long = NAME_ISSUE(rawValue: 1 << 5)
    public static let win_path_too_long = NAME_ISSUE(rawValue: 1 << 6)
    public static let mac_forbidden_char = NAME_ISSUE(rawValue: 1 << 7)
    public static let mac_name_too_long = NAME_ISSUE(rawValue: 1 << 8)
    public static let hidden = NAME_ISSUE(rawValue: 1 << 9)

    public let rawValue: UInt32

    public init(rawValue: UInt32)
    {
        self.rawValue = rawValue
    }
}
