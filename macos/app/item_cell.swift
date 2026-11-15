import AppKit

import limenamer_view_model

final class ITEM_CELL: NSTableCellView
{
    private static let PADDING: CGFloat = 8
    private static let TAG_GAP: CGFloat = 6
    private static let EDITOR_OUTSET: CGFloat = 3
    private static let DIM_ALPHA: CGFloat = 0.45
    private static let NAME_FONT: NSFont = NSFont.monospacedSystemFont(ofSize: 13, weight: .regular)
    private static let INDEX_FONT: NSFont = NSFont.monospacedDigitSystemFont(ofSize: 13,
        weight: .regular)
    private static let SMALL_FONT: NSFont = NSFont.monospacedDigitSystemFont(ofSize: 12,
        weight: .regular)

    private let column: ITEM_COLUMN
    private let label: NSTextField = NSTextField(labelWithString: "")
    private let badge: STATUS_BADGE = STATUS_BADGE()
    private let check_box: NSButton = NSButton(checkboxWithTitle: "", target: nil, action: nil)
    private let chip: TAG_VIEW?
    private var shows_chip: Bool = false
    private var tinted: Bool = false
    private(set) var editor: NSTextField? = nil

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
        switch column {
        case .check:
            addSubview(check_box)
        case .status:
            addSubview(badge)
        default:
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

    func bind_check(target: AnyObject, action: Selector)
    {
        check_box.target = target
        check_box.action = action
    }

    func show(_ row: ITEM_ROW)
    {
        switch column {
        case .check:
            break
        case .index:
            label.stringValue = String(row.index)
        case .status:
            badge.show(row.status, text: row.status_text)
        case .current:
            label.stringValue = row.name
            shows_chip = row.is_dir
            toolTip = row.full_path
        case .new_name:
            label.attributedStringValue = ITEM_CELL.new_name_text(row)
            shows_chip = row.direct
            toolTip = row.has_problems ? row.problem_text : nil
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
        chip?.isHidden = !shows_chip || editor != nil
        show_check(row)
        needsLayout = true
    }

    func show_check(_ row: ITEM_ROW)
    {
        if column == .check {
            check_box.state = row.is_checked ? .on : .off
        } else {
            alphaValue = row.is_dimmed ? ITEM_CELL.DIM_ALPHA : 1
        }
    }

    func begin_edit(_ text: String, delegate: NSTextFieldDelegate) -> NSTextField
    {
        let field: NSTextField = editor ?? ITEM_CELL.make_editor()
        field.stringValue = text
        field.delegate = delegate
        if field.superview == nil {
            addSubview(field)
        }
        editor = field
        label.isHidden = true
        chip?.isHidden = true
        needsLayout = true
        layoutSubtreeIfNeeded()
        return field
    }

    func end_edit()
    {
        guard let field = editor else {
            return
        }
        field.delegate = nil
        field.removeFromSuperview()
        editor = nil
        label.isHidden = false
        chip?.isHidden = !shows_chip
        needsLayout = true
    }

    override func layout()
    {
        super.layout()
        let left: CGFloat = ITEM_CELL.PADDING
        let right: CGFloat = bounds.width - ITEM_CELL.PADDING
        switch column {
        case .check:
            let size: NSSize = check_box.intrinsicContentSize
            check_box.frame = NSRect(x: round((bounds.width - size.width) / 2),
                y: middle(size.height), width: size.width, height: size.height)
            return
        case .status:
            let size: NSSize = badge.intrinsicContentSize
            badge.frame = NSRect(x: left, y: middle(size.height),
                width: max(0, min(size.width, right - left)), height: size.height)
            return
        default:
            break
        }
        if let editor {
            let height: CGFloat = editor.intrinsicContentSize.height + 2
            editor.frame = NSRect(x: left, y: middle(height),
                width: max(0, right - left + ITEM_CELL.EDITOR_OUTSET), height: height)
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

    private static func make_editor() -> NSTextField
    {
        let field: NSTextField = NSTextField(string: "")
        field.font = NAME_FONT
        field.isBordered = false
        field.isBezeled = false
        field.drawsBackground = true
        field.backgroundColor = .textBackgroundColor
        field.focusRingType = .exterior
        field.usesSingleLineMode = true
        field.maximumNumberOfLines = 1
        field.lineBreakMode = .byClipping
        field.cell?.wraps = false
        field.cell?.isScrollable = true
        return field
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
