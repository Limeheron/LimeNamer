import AppKit

import limenamer_view_model

final class MOD_CARD_VIEW: NSTableCellView
{
    static let IDENTIFIER: NSUserInterfaceItemIdentifier = NSUserInterfaceItemIdentifier("card")
    static let SIDE: CGFloat = 12
    static let GAP: CGFloat = 6
    private static let PAD_Y: CGFloat = 9
    private static let PAD_LEFT: CGFloat = 12
    private static let PAD_RIGHT: CGFloat = 8
    private static let CHECK_WIDTH: CGFloat = 14
    private static let REMOVE_SIZE: CGFloat = 20
    private static let SPACING: CGFloat = 10
    private static let RADIUS: CGFloat = 10
    private static let MIN_TEXT_HEIGHT: CGFloat = 20
    private static let OFF_ALPHA: CGFloat = 0.5
    private static let FONT: NSFont = NSFont.systemFont(ofSize: 13)
    private static let MEASURE: NSTextField = MOD_CARD_VIEW.make_label()

    var toggled: () -> Void = {}
    var removed: () -> Void = {}
    private let check_box: NSButton = NSButton(checkboxWithTitle: "", target: nil, action: nil)
    private let label: NSTextField = MOD_CARD_VIEW.make_label()
    private let remove_button: NSButton = NSButton()
    private let check_target: ACTION_TARGET = ACTION_TARGET()
    private let remove_target: ACTION_TARGET = ACTION_TARGET()
    private var hovering: Bool = false

    init()
    {
        super.init(frame: .zero)
        identifier = MOD_CARD_VIEW.IDENTIFIER
        check_box.toolTip = STRINGS.mod_toggle_tip
        check_box.target = check_target
        check_box.action = #selector(ACTION_TARGET.fire(_:))
        check_target.action = { [weak self] in
            self?.toggled()
        }
        let symbol: NSImage.SymbolConfiguration = NSImage.SymbolConfiguration(pointSize: 13,
            weight: .regular)
        remove_button.image = NSImage(systemSymbolName: "xmark.circle",
            accessibilityDescription: STRINGS.mod_remove_tip)?.withSymbolConfiguration(symbol)
        remove_button.isBordered = false
        remove_button.contentTintColor = .tertiaryLabelColor
        remove_button.toolTip = STRINGS.mod_remove_tip
        remove_button.isHidden = true
        remove_button.target = remove_target
        remove_button.action = #selector(ACTION_TARGET.fire(_:))
        remove_target.action = { [weak self] in
            self?.removed()
        }
        addSubview(check_box)
        addSubview(label)
        addSubview(remove_button)
    }

    required init?(coder: NSCoder)
    {
        fatalError("MOD_CARD_VIEW does not support coding")
    }

    static func height(_ summary: String, width: CGFloat) -> CGFloat
    {
        MEASURE.stringValue = summary
        let text_width: CGFloat = max(1, text_width(width))
        let size: NSSize = MEASURE.cell?.cellSize(forBounds: NSRect(x: 0, y: 0,
            width: text_width, height: CGFloat.greatestFiniteMagnitude)) ?? .zero
        return max(ceil(size.height), MIN_TEXT_HEIGHT) + PAD_Y * 2 + GAP
    }

    func show(_ card: MOD_CARD, enabled: Bool)
    {
        check_box.state = card.enabled ? .on : .off
        check_box.isEnabled = enabled
        remove_button.isEnabled = enabled
        label.stringValue = card.summary
        label.alphaValue = card.enabled ? 1 : MOD_CARD_VIEW.OFF_ALPHA
        let inside: Bool = window.map { window in
            return bounds.contains(convert(window.mouseLocationOutsideOfEventStream, from: nil))
        } ?? false
        set_hovering(inside)
        needsLayout = true
    }

    override func setFrameSize(_ size: NSSize)
    {
        super.setFrameSize(size)
        needsLayout = true
    }

    override func updateTrackingAreas()
    {
        super.updateTrackingAreas()
        for area in trackingAreas where area.owner === self {
            removeTrackingArea(area)
        }
        addTrackingArea(NSTrackingArea(rect: .zero,
            options: [.mouseEnteredAndExited, .activeInKeyWindow, .inVisibleRect],
            owner: self, userInfo: nil))
    }

    override func mouseEntered(with event: NSEvent)
    {
        set_hovering(true)
    }

    override func mouseExited(with event: NSEvent)
    {
        set_hovering(false)
    }

    override func layout()
    {
        super.layout()
        let card: NSRect = card_rect()
        let check: NSSize = check_box.intrinsicContentSize
        check_box.frame = NSRect(x: card.minX + MOD_CARD_VIEW.PAD_LEFT,
            y: round(card.midY - check.height / 2), width: check.width, height: check.height)
        let size: CGFloat = MOD_CARD_VIEW.REMOVE_SIZE
        remove_button.frame = NSRect(x: card.maxX - MOD_CARD_VIEW.PAD_RIGHT - size,
            y: round(card.midY - size / 2), width: size, height: size)
        let left: CGFloat = card.minX + MOD_CARD_VIEW.PAD_LEFT + MOD_CARD_VIEW.CHECK_WIDTH
            + MOD_CARD_VIEW.SPACING
        let width: CGFloat = max(1, MOD_CARD_VIEW.text_width(bounds.width))
        let height: CGFloat = label.cell?.cellSize(forBounds: NSRect(x: 0, y: 0, width: width,
            height: CGFloat.greatestFiniteMagnitude)).height ?? 0
        label.frame = NSRect(x: left, y: round(card.midY - height / 2), width: width,
            height: ceil(height))
    }

    override func draw(_ dirty_rect: NSRect)
    {
        let rect: NSRect = card_rect().insetBy(dx: 0.5, dy: 0.5)
        let path: NSBezierPath = NSBezierPath(roundedRect: rect, xRadius: MOD_CARD_VIEW.RADIUS,
            yRadius: MOD_CARD_VIEW.RADIUS)
        NSGraphicsContext.saveGraphicsState()
        let shadow: NSShadow = NSShadow()
        shadow.shadowOffset = NSSize(width: 0, height: -1)
        shadow.shadowBlurRadius = hovering ? 3 : 1
        shadow.shadowColor = NSColor.black.withAlphaComponent(hovering ? 0.08 : 0.04)
        shadow.set()
        (hovering ? PALETTE.CARD_HOVER_BACK : PALETTE.CARD_BACK).setFill()
        path.fill()
        NSGraphicsContext.restoreGraphicsState()
        (hovering ? PALETTE.CARD_HOVER_LINE : PALETTE.CARD_LINE).setStroke()
        path.lineWidth = 1
        path.stroke()
    }

    private func set_hovering(_ value: Bool)
    {
        if hovering != value {
            hovering = value
            needsDisplay = true
        }
        remove_button.isHidden = !value
    }

    private func card_rect() -> NSRect
    {
        return bounds.insetBy(dx: MOD_CARD_VIEW.SIDE, dy: MOD_CARD_VIEW.GAP / 2)
    }

    private static func text_width(_ width: CGFloat) -> CGFloat
    {
        return width - SIDE * 2 - PAD_LEFT - CHECK_WIDTH - SPACING * 2 - REMOVE_SIZE - PAD_RIGHT
    }

    private static func make_label() -> NSTextField
    {
        let label: NSTextField = NSTextField(wrappingLabelWithString: "")
        label.font = FONT
        label.textColor = .labelColor
        label.isSelectable = false
        label.lineBreakStrategy = .hangulWordPriority
        return label
    }
}
