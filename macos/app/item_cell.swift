import AppKit

import limenamer_view_model

final class ITEM_CELL: NSTableCellView
{
    private static let PADDING: CGFloat = 8
    private static let TAG_GAP: CGFloat = 6
    private static let NAME_FONT: NSFont = NSFont.monospacedSystemFont(ofSize: 13, weight: .regular)
    private static let INDEX_FONT: NSFont = NSFont.monospacedDigitSystemFont(ofSize: 13,
        weight: .regular)
    private static let SMALL_FONT: NSFont = NSFont.monospacedDigitSystemFont(ofSize: 12,
        weight: .regular)

    private let column: ITEM_COLUMN
    private let label: NSTextField = NSTextField(labelWithString: "")
    private let badge: STATUS_BADGE = STATUS_BADGE()
    private let chip: TAG_VIEW?
    private var tinted: Bool = false

    init(column: ITEM_COLUMN)
    {
        self.column = column
        switch column {
        case .current:
            chip = TAG_VIEW(text: STRINGS.tag_folder, text_color: PALETTE.FOLDER_TEXT,
                back_color: PALETTE.FOLDER_BACK)
        case .new_name:
            chip = TAG_VIEW(text: STRINGS.tag_direct, text_color: PALETTE.ACCENT_TEXT,
                back_color: PALETTE.ACCENT_SOFT)
        default:
            chip = nil
        }
        super.init(frame: .zero)
        identifier = column.identifier
        if column == .status {
            addSubview(badge)
        } else {
            setup_label()
            addSubview(label)
            textField = label
        }
        if let chip {
            chip.isHidden = true
            addSubview(chip)
        }
    }

    required init?(coder: NSCoder)
    {
        fatalError("ITEM_CELL does not support coding")
    }

    func show(_ row: ITEM_ROW)
    {
        switch column {
        case .index:
            label.stringValue = String(row.index)
        case .status:
            badge.show(row.status, text: row.status_text)
        case .current:
            label.stringValue = row.name
            chip?.isHidden = !row.is_dir
            toolTip = row.full_path
        case .new_name:
            label.attributedStringValue = ITEM_CELL.new_name_text(row)
            chip?.isHidden = !row.direct
            if tinted != row.changed {
                tinted = row.changed
                needsDisplay = true
            }
        case .folder:
            label.stringValue = row.folder
            toolTip = row.folder
        case .modified:
            label.stringValue = row.modified_text
        case .created:
            label.stringValue = row.created_text
        case .size:
            label.stringValue = row.size_text
        }
        needsLayout = true
    }

    override func layout()
    {
        super.layout()
        let left: CGFloat = ITEM_CELL.PADDING
        let right: CGFloat = bounds.width - ITEM_CELL.PADDING
        if column == .status {
            let size: NSSize = badge.intrinsicContentSize
            badge.frame = NSRect(x: left, y: middle(size.height),
                width: max(0, min(size.width, right - left)), height: size.height)
            return
        }
        var text_left: CGFloat = left
        var text_right: CGFloat = right
        if let chip, !chip.isHidden {
            let size: NSSize = chip.intrinsicContentSize
            if column == .current {
                chip.frame = NSRect(x: left, y: middle(size.height), width: size.width,
                    height: size.height)
                text_left = left + size.width + ITEM_CELL.TAG_GAP
            } else {
                let room: CGFloat = right - left - size.width - ITEM_CELL.TAG_GAP
                let natural: CGFloat = label.cell?.cellSize.width
                    ?? label.intrinsicContentSize.width
                let width: CGFloat = max(0, min(ceil(natural), room))
                text_right = left + width
                chip.frame = NSRect(x: text_right + ITEM_CELL.TAG_GAP, y: middle(size.height),
                    width: size.width, height: size.height)
            }
        }
        let height: CGFloat = label.intrinsicContentSize.height
        label.frame = NSRect(x: text_left, y: middle(height),
            width: max(0, text_right - text_left), height: height)
    }

    override func draw(_ dirty_rect: NSRect)
    {
        if tinted {
            PALETTE.CHANGED_BACK.setFill()
            bounds.fill(using: .sourceOver)
        }
    }

    private func middle(_ height: CGFloat) -> CGFloat
    {
        return round((bounds.height - height) / 2)
    }

    private func setup_label()
    {
        label.lineBreakMode = .byTruncatingTail
        label.maximumNumberOfLines = 1
        label.alignment = column.alignment
        label.textColor = .secondaryLabelColor
        switch column {
        case .index:
            label.font = ITEM_CELL.INDEX_FONT
        case .current, .new_name:
            label.font = ITEM_CELL.NAME_FONT
        case .folder:
            label.font = NSFont.systemFont(ofSize: 13)
            label.lineBreakMode = .byTruncatingMiddle
        default:
            label.font = ITEM_CELL.SMALL_FONT
        }
    }

    private static func new_name_text(_ row: ITEM_ROW) -> NSAttributedString
    {
        let paragraph: NSMutableParagraphStyle = NSMutableParagraphStyle()
        paragraph.lineBreakMode = .byTruncatingTail
        let color: NSColor = row.status == .ok ? .labelColor : STATUS_LOOK.text(row.status)
        let base: [NSAttributedString.Key: Any] = [.font: NAME_FONT, .foregroundColor: color,
            .paragraphStyle: paragraph]
        let parts: (head: String, mark: String, tail: String) = row.new_name_parts
        let text: NSMutableAttributedString = NSMutableAttributedString(string: parts.head,
            attributes: base)
        var marked: [NSAttributedString.Key: Any] = base
        marked[.backgroundColor] = PALETTE.MARK_BACK
        text.append(NSAttributedString(string: parts.mark, attributes: marked))
        text.append(NSAttributedString(string: parts.tail, attributes: base))
        return text
    }
}
