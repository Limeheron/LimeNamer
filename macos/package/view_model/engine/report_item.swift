public struct REPORT_ITEM: Equatable, Sendable
{
    public let item_id: UInt64
    public let from_path: String
    public let to_path: String
    public let current_path: String
    public let stage: STAGE
    public let failure: FAILURE
    public let os_error: Int32

    public init(item_id: UInt64, from_path: String, to_path: String, current_path: String,
        stage: STAGE, failure: FAILURE, os_error: Int32)
    {
        self.item_id = item_id
        self.from_path = from_path
        self.to_path = to_path
        self.current_path = current_path
        self.stage = stage
        self.failure = failure
        self.os_error = os_error
    }
}
