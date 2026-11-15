import AppKit

import limenamer_view_model

final class TOKEN_CHIP_CELL: NSTextAttachmentCell
{
    nonisolated private static var FONT: NSFont
    {
        return NSFont.systemFont(ofSize: 12, weight: .semibold)
    }

    nonisolated private static let HEIGHT: CGFloat = 22
    nonisolated private static let PAD_LEFT: CGFloat = 8
    nonisolated private static let PAD_RIGHT: CGFloat = 5
    nonisolated private static let GAP: CGFloat = 3
    nonisolated private static let MARGIN: CGFloat = 2
    nonisolated private static let CROSS: CGFloat = 10
    nonisolated private static let CROSS_HIT: CGFloat = 18
    nonisolated private static let BASELINE: CGFloat = -7

    let token: TOKEN_KIND
    private let name: String

    init(token: TOKEN_KIND)
    {
        self.token = token
        name = TOKEN_CHIP_CELL.name(token)
        super.init(textCell: "")
    }

    required init(coder: NSCoder)
    {
        fatalError("TOKEN_CHIP_CELL does not support coding")
    }

    static func name(_ token: TOKEN_KIND) -> String
    {
        switch token {
        case .number:
            return STRINGS.token_number
        case .original_name:
            return STRINGS.token_original_name
        case .modified_date:
            return STRINGS.token_modified_date
        case .created_date:
            return STRINGS.token_created_date
        case .size:
            return STRINGS.token_size
        case .parent_folder:
            return STRINGS.token_parent_folder
        }
    }

    override func cellSize() -> NSSize
    {
        let text: NSSize = (name as NSString).size(withAttributes: [.font: TOKEN_CHIP_CELL.FONT])
        let width: CGFloat = TOKEN_CHIP_CELL.MARGIN * 2 + TOKEN_CHIP_CELL.PAD_LEFT
            + ceil(text.width) + TOKEN_CHIP_CELL.GAP + TOKEN_CHIP_CELL.CROSS
            + TOKEN_CHIP_CELL.PAD_RIGHT
        return NSSize(width: width, height: TOKEN_CHIP_CELL.HEIGHT)
    }

    override func cellBaselineOffset() -> NSPoint
    {
        return NSPoint(x: 0, y: TOKEN_CHIP_CELL.BASELINE)
    }

    override func draw(withFrame cell_frame: NSRect, in control_view: NSView?)
    {
        let chip: NSRect = cell_frame.insetBy(dx: TOKEN_CHIP_CELL.MARGIN, dy: 0)
        let radius: CGFloat = chip.height / 2
        PALETTE.ACCENT_SOFT.setFill()
        NSBezierPath(roundedRect: chip, xRadius: radius, yRadius: radius).fill()
        let attributes: [NSAttributedString.Key: Any] = [.font: TOKEN_CHIP_CELL.FONT,
            .foregroundColor: PALETTE.ACCENT_TEXT]
        let size: NSSize = (name as NSString).size(withAttributes: attributes)
        (name as NSString).draw(at: NSPoint(x: chip.minX + TOKEN_CHIP_CELL.PAD_LEFT,
            y: chip.midY - size.height / 2), withAttributes: attributes)
        let cross: NSRect = cross_rect(chip)
        let path: NSBezierPath = NSBezierPath()
        let inset: CGFloat = 2
        path.move(to: NSPoint(x: cross.minX + inset, y: cross.minY + inset))
        path.line(to: NSPoint(x: cross.maxX - inset, y: cross.maxY - inset))
        path.move(to: NSPoint(x: cross.maxX - inset, y: cross.minY + inset))
        path.line(to: NSPoint(x: cross.minX + inset, y: cross.maxY - inset))
        path.lineWidth = 1.5
        path.lineCapStyle = .round
        PALETTE.ACCENT_TEXT.setStroke()
        path.stroke()
    }

    override func wantsToTrackMouse() -> Bool
    {
        return true
    }

    override func trackMouse(with event: NSEvent, in cell_frame: NSRect, of control_view: NSView?,
        atCharacterIndex char_index: Int, untilMouseUp flag: Bool) -> Bool
    {
        guard let text_view = control_view as? TOKEN_TEXT_VIEW else {
            return false
        }
        let point: NSPoint = text_view.convert(event.locationInWindow, from: nil)
        let chip: NSRect = cell_frame.insetBy(dx: TOKEN_CHIP_CELL.MARGIN, dy: 0)
        let cross: NSRect = cross_rect(chip)
        let hit: NSRect = NSRect(x: cross.midX - TOKEN_CHIP_CELL.CROSS_HIT / 2, y: chip.minY,
            width: TOKEN_CHIP_CELL.CROSS_HIT, height: chip.height)
        if !hit.contains(point) {
            return false
        }
        text_view.remove_token(at: char_index)
        return true
    }

    private func cross_rect(_ chip: NSRect) -> NSRect
    {
        let size: CGFloat = TOKEN_CHIP_CELL.CROSS
        return NSRect(x: chip.maxX - TOKEN_CHIP_CELL.PAD_RIGHT - size,
            y: chip.midY - size / 2, width: size, height: size)
    }
}
