public enum FAILURE: Int32, Sendable
{
    case none = 0
    case in_use = 1
    case access_denied = 2
    case not_found = 3
    case already_exists = 4
    case replaced = 5
    case path_too_long = 6
    case depends_on_failed = 7
    case other = 8
}
