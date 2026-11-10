public struct ENGINE_ERROR: Error, Equatable, CustomStringConvertible
{
    public let result: RESULT

    public init(result: RESULT)
    {
        self.result = result
    }

    public var description: String
    {
        return "Engine call failed: \(result)"
    }
}
