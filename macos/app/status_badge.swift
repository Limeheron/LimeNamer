import AppKit

import limenamer_view_model

final class STATUS_BADGE: NSView
{
    private static let FONT: NSFont = NSFont.systemFont(ofSize: 12)
    private static let HEIGHT: CGFloat = 20
    private static let ICON_SIZE: CGFloat = 12
    private static let LEFT: CGFloat = 6
    private static let RIGHT: CGFloat = 8
    private static let GAP: CGFloat = 4

    private var status: STATUS = .ok
    private var text: String = ""
    private var icon: NSImage? = nil

    init()
    {
        super.init(frame: .zero)
        setAccessibilityElement(true)
        setAccessibilityRole(.staticText)
    }

    required init?(coder: NSCoder)
    {
        fatalError("STATUS_BADGE does not support coding")
    }

    override var intrinsicContentSize: NSSize
    {
        let width: CGFloat = STATUS_BADGE.LEFT + STATUS_BADGE.ICON_SIZE + STATUS_BADGE.GAP
            + ceil(text_size().width) + STATUS_BADGE.RIGHT
        return NSSize(width: width, height: STATUS_BADGE.HEIGHT)
    }

    func show(_ status: STATUS, text: String)
    {
        if status == self.status && text == self.text && icon != nil {
            return
        }
        self.status = status
        self.text = text
        let configuration: NSImage.SymbolConfiguration = NSImage.SymbolConfiguration(
            pointSize: STATUS_BADGE.ICON_SIZE - 1, weight: .medium)
        icon = NSImage(systemSymbolName: STATUS_LOOK.symbol(status),
            accessibilityDescription: nil)?.withSymbolConfiguration(configuration)
        setAccessibilityLabel(text)
        invalidateIntrinsicContentSize()
        needsDisplay = true
    }

    override func draw(_ dirty_rect: NSRect)
    {
        let radius: CGFloat = bounds.height / 2
        STATUS_LOOK.back(status).setFill()
        NSBezierPath(roundedRect: bounds, xRadius: radius, yRadius: radius).fill()
        let color: NSColor = STATUS_LOOK.text(status)
        if let icon {
            let size: NSSize = STATUS_BADGE.fit(icon.size, in: STATUS_BADGE.ICON_SIZE)
            let rect: NSRect = NSRect(
                x: STATUS_BADGE.LEFT + (STATUS_BADGE.ICON_SIZE - size.width) / 2,
                y: (bounds.height - size.height) / 2, width: size.width, height: size.height)
            STATUS_BADGE.draw_tinted(icon, in: rect, color: color)
        }
        let size: NSSize = text_size()
        let origin: NSPoint = NSPoint(
            x: STATUS_BADGE.LEFT + STATUS_BADGE.ICON_SIZE + STATUS_BADGE.GAP,
            y: round((bounds.height - size.height) / 2))
        (text as NSString).draw(at: origin, withAttributes: [.font: STATUS_BADGE.FONT,
            .foregroundColor: color])
    }

    private func text_size() -> NSSize
    {
        return (text as NSString).size(withAttributes: [.font: STATUS_BADGE.FONT])
    }

    private static func fit(_ size: NSSize, in side: CGFloat) -> NSSize
    {
        let scale: CGFloat = min(1, side / max(size.width, size.height, 1))
        return NSSize(width: size.width * scale, height: size.height * scale)
    }

    private static func draw_tinted(_ image: NSImage, in rect: NSRect, color: NSColor)
    {
        guard let context: CGContext = NSGraphicsContext.current?.cgContext else {
            return
        }
        context.saveGState()
        context.beginTransparencyLayer(in: rect, auxiliaryInfo: nil)
        image.draw(in: rect)
        color.setFill()
        rect.fill(using: .sourceAtop)
        context.endTransparencyLayer()
        context.restoreGState()
    }
}
