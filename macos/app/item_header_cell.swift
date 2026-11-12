import AppKit

final class ITEM_HEADER_CELL: NSTableHeaderCell
{
    private static let INSET: CGFloat = 6

    override func drawInterior(withFrame cell_frame: NSRect, in control_view: NSView)
    {
        super.drawInterior(withFrame: cell_frame.insetBy(dx: ITEM_HEADER_CELL.INSET, dy: 0),
            in: control_view)
    }
}
