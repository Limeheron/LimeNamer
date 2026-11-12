import AppKit

final class ITEM_ROW_VIEW: NSTableRowView
{
    private static let RADIUS: CGFloat = 6
    private static let BAR_LEFT: CGFloat = 3
    private static let BAR_WIDTH: CGFloat = 3
    private static let BAR_INSET: CGFloat = 7

    override var interiorBackgroundStyle: NSView.BackgroundStyle
    {
        return .normal
    }

    override func drawSelection(in dirty_rect: NSRect)
    {
        let round_previous: Bool = !isPreviousRowSelected
        let round_next: Bool = !isNextRowSelected
        let round_min: Bool = isFlipped ? round_previous : round_next
        let round_max: Bool = isFlipped ? round_next : round_previous
        PALETTE.SELECTED_BACK.setFill()
        ITEM_ROW_VIEW.path(bounds, round_min: round_min, round_max: round_max).fill()
        let bar: NSRect = NSRect(x: ITEM_ROW_VIEW.BAR_LEFT, y: ITEM_ROW_VIEW.BAR_INSET,
            width: ITEM_ROW_VIEW.BAR_WIDTH,
            height: max(0, bounds.height - ITEM_ROW_VIEW.BAR_INSET * 2))
        let bar_radius: CGFloat = ITEM_ROW_VIEW.BAR_WIDTH / 2
        PALETTE.ACCENT_TEXT.setFill()
        NSBezierPath(roundedRect: bar, xRadius: bar_radius, yRadius: bar_radius).fill()
    }

    private static func path(_ rect: NSRect, round_min: Bool, round_max: Bool) -> NSBezierPath
    {
        let min_radius: CGFloat = round_min ? RADIUS : 0
        let max_radius: CGFloat = round_max ? RADIUS : 0
        let path: NSBezierPath = NSBezierPath()
        path.move(to: NSPoint(x: rect.minX + min_radius, y: rect.minY))
        path.appendArc(from: NSPoint(x: rect.maxX, y: rect.minY),
            to: NSPoint(x: rect.maxX, y: rect.maxY), radius: min_radius)
        path.appendArc(from: NSPoint(x: rect.maxX, y: rect.maxY),
            to: NSPoint(x: rect.minX, y: rect.maxY), radius: max_radius)
        path.appendArc(from: NSPoint(x: rect.minX, y: rect.maxY),
            to: NSPoint(x: rect.minX, y: rect.minY), radius: max_radius)
        path.appendArc(from: NSPoint(x: rect.minX, y: rect.minY),
            to: NSPoint(x: rect.maxX, y: rect.minY), radius: min_radius)
        path.close()
        return path
    }
}
