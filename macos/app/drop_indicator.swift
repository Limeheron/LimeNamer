import AppKit

@MainActor
final class DROP_INDICATOR
{
    private let table: NSTableView
    private let scroll_view: NSScrollView
    private let left: CGFloat
    private let right: CGFloat
    private let line: DROP_LINE_VIEW = DROP_LINE_VIEW()

    init(table: NSTableView, scroll_view: NSScrollView, left: CGFloat, right: CGFloat)
    {
        self.table = table
        self.scroll_view = scroll_view
        self.left = left
        self.right = right
        line.isHidden = true
    }

    func gap(at window_point: NSPoint) -> Int
    {
        let count: Int = table.numberOfRows
        if count == 0 {
            return 0
        }
        let clip: NSClipView = scroll_view.contentView
        let top: CGFloat = clip.bounds.minY + clip.contentInsets.top
        let bottom: CGFloat = clip.bounds.maxY - clip.contentInsets.bottom
        let point: NSPoint = table.convert(window_point, from: nil)
        let y: CGFloat = min(max(point.y, top + 1), bottom - 1)
        let row: Int = table.row(at: NSPoint(x: table.visibleRect.midX, y: y))
        var gap: Int = count
        if row >= 0 {
            gap = y > table.rect(ofRow: row).midY ? row + 1 : row
        }
        if gap < count && gap_y(gap) < top - 1 {
            gap += 1
        } else if gap > 0 && gap_y(gap) > bottom + 1 {
            gap -= 1
        }
        return gap
    }

    func show(_ gap: Int)
    {
        if table.numberOfRows == 0 {
            hide()
            return
        }
        let half: CGFloat = DROP_LINE_VIEW.HEIGHT / 2
        let y: CGFloat = min(max(gap_y(gap), half), max(half, table.bounds.height - half))
        let width: CGFloat = max(0, table.bounds.width - left - right)
        line.frame = NSRect(x: left, y: y - half, width: width, height: DROP_LINE_VIEW.HEIGHT)
        if table.subviews.last !== line {
            table.addSubview(line, positioned: .above, relativeTo: nil)
        }
        line.isHidden = false
    }

    func hide()
    {
        line.isHidden = true
    }

    private func gap_y(_ gap: Int) -> CGFloat
    {
        let count: Int = table.numberOfRows
        return gap < count ? table.rect(ofRow: gap).minY : table.rect(ofRow: count - 1).maxY
    }
}
