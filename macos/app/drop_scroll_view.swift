import AppKit

final class DROP_SCROLL_VIEW: NSScrollView
{
    var drag_updated: ((any NSDraggingInfo) -> NSDragOperation)? = nil
    var drag_exited: (() -> Void)? = nil
    var drag_performed: ((any NSDraggingInfo) -> Bool)? = nil

    override func draggingEntered(_ sender: any NSDraggingInfo) -> NSDragOperation
    {
        return drag_updated?(sender) ?? []
    }

    override func draggingUpdated(_ sender: any NSDraggingInfo) -> NSDragOperation
    {
        return drag_updated?(sender) ?? []
    }

    override func draggingExited(_ sender: (any NSDraggingInfo)?)
    {
        drag_exited?()
    }

    override func performDragOperation(_ sender: any NSDraggingInfo) -> Bool
    {
        return drag_performed?(sender) ?? false
    }
}
