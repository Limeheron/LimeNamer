import os

public final class PROGRESS: Sendable
{
    private let value = OSAllocatedUnfairLock<(done: UInt32, total: UInt32)>(
        initialState: (0, 0))

    public init()
    {
    }

    public func read() -> (done: UInt32, total: UInt32)
    {
        return value.withLock { $0 }
    }

    func set(_ done: UInt32, _ total: UInt32)
    {
        value.withLock { $0 = (done, total) }
    }
}
