import AppKit

final class DROP_LINE_VIEW: NSView
{
    static let HEIGHT: CGFloat = 8
    private static let LINE_LEFT: CGFloat = 6
    private static let LINE_HEIGHT: CGFloat = 2
    private static let STROKE: CGFloat = 2

    override func draw(_ dirty_rect: NSRect)
    {
        let middle: CGFloat = bounds.midY
        let line: NSRect = NSRect(x: DROP_LINE_VIEW.LINE_LEFT,
            y: middle - DROP_LINE_VIEW.LINE_HEIGHT / 2,
            width: max(0, bounds.width - DROP_LINE_VIEW.LINE_LEFT),
            height: DROP_LINE_VIEW.LINE_HEIGHT)
        let radius: CGFloat = DROP_LINE_VIEW.LINE_HEIGHT / 2
        PALETTE.ACCENT_TEXT.setFill()
        NSBezierPath(roundedRect: line, xRadius: radius, yRadius: radius).fill()
        let inset: CGFloat = DROP_LINE_VIEW.STROKE / 2
        let top: CGFloat = middle - DROP_LINE_VIEW.HEIGHT / 2
        let dot: NSBezierPath = NSBezierPath(ovalIn: NSRect(x: inset, y: top + inset,
            width: DROP_LINE_VIEW.HEIGHT - DROP_LINE_VIEW.STROKE,
            height: DROP_LINE_VIEW.HEIGHT - DROP_LINE_VIEW.STROKE))
        NSColor.controlBackgroundColor.setFill()
        dot.fill()
        PALETTE.ACCENT_TEXT.setStroke()
        dot.lineWidth = DROP_LINE_VIEW.STROKE
        dot.stroke()
    }

    override func hitTest(_ point: NSPoint) -> NSView?
    {
        return nil
    }
}
