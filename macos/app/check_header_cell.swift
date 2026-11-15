import AppKit

final class CHECK_HEADER_CELL: NSTableHeaderCell
{
    var check_state: NSControl.StateValue = .off

    override func drawInterior(withFrame cell_frame: NSRect, in control_view: NSView)
    {
        let box: NSButtonCell = NSButtonCell()
        box.setButtonType(.switch)
        box.title = ""
        box.allowsMixedState = true
        box.state = check_state
        let size: NSSize = box.cellSize
        let rect: NSRect = NSRect(x: round(cell_frame.midX - size.width / 2),
            y: round(cell_frame.midY - size.height / 2), width: size.width, height: size.height)
        box.draw(withFrame: rect, in: control_view)
    }
}
