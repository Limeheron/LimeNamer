public enum RESULT: Int32, Sendable
{
    case ok = 0
    case invalid_argument = 1
    case invalid_json = 2
    case not_found = 3
    case busy = 4
    case not_ready = 5
    case io = 6
    case out_of_memory = 7
    case internal_error = 8
}
