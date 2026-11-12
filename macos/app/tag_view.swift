import AppKit

final class TAG_VIEW: NSView
{
    private static let FONT: NSFont = NSFont.systemFont(ofSize: 11)
    private static let HEIGHT: CGFloat = 16
    private static let PADDING: CGFloat = 6

    private let text: String
    private let text_color: NSColor
    private let back_color: NSColor

    init(text: String, text_color: NSColor, back_color: NSColor)
    {
        self.text = text
        self.text_color = text_color
        self.back_color = back_color
        super.init(frame: .zero)
        setAccessibilityElement(true)
        setAccessibilityRole(.staticText)
        setAccessibilityLabel(text)
    }

    required init?(coder: NSCoder)
    {
        fatalError("TAG_VIEW does not support coding")
    }

    override var intrinsicContentSize: NSSize
    {
        let width: CGFloat = ceil(text_size().width) + TAG_VIEW.PADDING * 2
        return NSSize(width: width, height: TAG_VIEW.HEIGHT)
    }

    override func draw(_ dirty_rect: NSRect)
    {
        let radius: CGFloat = bounds.height / 2
        back_color.setFill()
        NSBezierPath(roundedRect: bounds, xRadius: radius, yRadius: radius).fill()
        let size: NSSize = text_size()
        let origin: NSPoint = NSPoint(x: round((bounds.width - size.width) / 2),
            y: round((bounds.height - size.height) / 2))
        (text as NSString).draw(at: origin, withAttributes: attributes())
    }

    private func text_size() -> NSSize
    {
        return (text as NSString).size(withAttributes: attributes())
    }

    private func attributes() -> [NSAttributedString.Key: Any]
    {
        return [.font: TAG_VIEW.FONT, .foregroundColor: text_color]
    }
}
