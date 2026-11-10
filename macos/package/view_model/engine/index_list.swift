public struct INDEX_LIST: Equatable, Sendable
{
    public let indices: [UInt32]
    public let error: INDEX_ERROR

    public init(indices: [UInt32], error: INDEX_ERROR)
    {
        self.indices = indices
        self.error = error
    }
}
