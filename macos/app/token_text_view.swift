import AppKit

import limenamer_view_model

final class TOKEN_TEXT_VIEW: NSTextView
{
    static let MIN_HEIGHT: CGFloat = 32
    private static let INSET: NSSize = NSSize(width: 10, height: 6)
    private static let FONT: NSFont = NSFont.monospacedSystemFont(ofSize: 14, weight: .regular)
    private static let ATTACHMENT_CHARACTER: Character = "\u{FFFC}"
    private static let NEWLINES: CharacterSet = .newlines

    var changed: ([VALUE_PIECE]) -> Void = { _ in }
    var focus_changed: (Bool) -> Void = { _ in }
    var submit: () -> Void = {}
    var cancel: () -> Void = {}
    private let owned_storage: NSTextStorage
    private let owned_undo: UndoManager = UndoManager()
    private var updating: Bool = false

    init()
    {
        let storage: NSTextStorage = NSTextStorage()
        let layout: NSLayoutManager = NSLayoutManager()
        storage.addLayoutManager(layout)
        let container: NSTextContainer = NSTextContainer(size: NSSize(width: 0,
            height: CGFloat.greatestFiniteMagnitude))
        container.widthTracksTextView = true
        container.lineFragmentPadding = 0
        layout.addTextContainer(container)
        owned_storage = storage
        super.init(frame: .zero, textContainer: container)
        setup()
    }

    required init?(coder: NSCoder)
    {
        fatalError("TOKEN_TEXT_VIEW does not support coding")
    }

    override var typingAttributes: [NSAttributedString.Key: Any]
    {
        get {
            return TOKEN_TEXT_VIEW.text_attributes()
        }
        set {
        }
    }

    override var readablePasteboardTypes: [NSPasteboard.PasteboardType]
    {
        return [.string]
    }

    override var writablePasteboardTypes: [NSPasteboard.PasteboardType]
    {
        return [.string]
    }

    override var acceptsFirstResponder: Bool
    {
        return isEditable
    }

    override var undoManager: UndoManager?
    {
        return owned_undo
    }

    override var intrinsicContentSize: NSSize
    {
        return NSSize(width: NSView.noIntrinsicMetric, height: NSView.noIntrinsicMetric)
    }

    static func normalize(_ pieces: [VALUE_PIECE]) -> [VALUE_PIECE]
    {
        var result: [VALUE_PIECE] = []
        for piece in pieces {
            if let token = piece.token {
                result.append(VALUE_PIECE(token: token))
            } else if let text = piece.text, !text.isEmpty {
                if let last = result.last, last.token == nil {
                    result[result.count - 1] = VALUE_PIECE(text: (last.text ?? "") + text)
                } else {
                    result.append(VALUE_PIECE(text: text))
                }
            }
        }
        return result
    }

    func pieces() -> [VALUE_PIECE]
    {
        var result: [VALUE_PIECE] = []
        let full: NSRange = NSRange(location: 0, length: owned_storage.length)
        let text: NSString = owned_storage.string as NSString
        owned_storage.enumerateAttribute(.attachment, in: full) { value, range, _ in
            if let attachment = value as? NSTextAttachment,
                let chip = attachment.attachmentCell as? TOKEN_CHIP_CELL {
                for _ in 0..<range.length {
                    result.append(VALUE_PIECE(token: chip.token))
                }
            } else {
                let part: String = text.substring(with: range)
                result.append(VALUE_PIECE(text: part.filter {
                    $0 != TOKEN_TEXT_VIEW.ATTACHMENT_CHARACTER
                }))
            }
        }
        return TOKEN_TEXT_VIEW.normalize(result)
    }

    func set_pieces(_ pieces: [VALUE_PIECE])
    {
        if TOKEN_TEXT_VIEW.normalize(pieces) == self.pieces() {
            return
        }
        let text: NSMutableAttributedString = NSMutableAttributedString()
        for piece in pieces {
            if let token = piece.token {
                text.append(TOKEN_TEXT_VIEW.chip(token))
            } else if let part = piece.text {
                text.append(NSAttributedString(string: part,
                    attributes: TOKEN_TEXT_VIEW.text_attributes()))
            }
        }
        updating = true
        owned_storage.setAttributedString(text)
        owned_undo.removeAllActions()
        setSelectedRange(NSRange(location: owned_storage.length, length: 0))
        updating = false
        invalidateIntrinsicContentSize()
    }

    func insert_token(_ token: TOKEN_KIND)
    {
        window?.makeFirstResponder(self)
        replace(selectedRange(), with: TOKEN_TEXT_VIEW.chip(token))
    }

    func remove_token(at index: Int)
    {
        if index >= 0 && index < owned_storage.length {
            replace(NSRange(location: index, length: 1), with: NSAttributedString())
        }
    }

    func height(for width: CGFloat) -> CGFloat
    {
        let total: CGFloat = width.isFinite && width > 0 ? width : bounds.width
        if total <= 0 {
            return TOKEN_TEXT_VIEW.MIN_HEIGHT
        }
        let storage: NSTextStorage = NSTextStorage(attributedString: owned_storage)
        let layout: NSLayoutManager = NSLayoutManager()
        storage.addLayoutManager(layout)
        let container: NSTextContainer = NSTextContainer(size: NSSize(
            width: max(1, total - TOKEN_TEXT_VIEW.INSET.width * 2),
            height: CGFloat.greatestFiniteMagnitude))
        container.lineFragmentPadding = 0
        layout.addTextContainer(container)
        layout.ensureLayout(for: container)
        let used: CGFloat = max(layout.usedRect(for: container).height,
            TOKEN_TEXT_VIEW.FONT.ascender - TOKEN_TEXT_VIEW.FONT.descender)
        return max(TOKEN_TEXT_VIEW.MIN_HEIGHT, ceil(used + TOKEN_TEXT_VIEW.INSET.height * 2))
    }

    override func shouldChangeText(in range: NSRange, replacementString string: String?) -> Bool
    {
        if let string, string.rangeOfCharacter(from: TOKEN_TEXT_VIEW.NEWLINES) != nil {
            let line: String = string.components(separatedBy: TOKEN_TEXT_VIEW.NEWLINES)
                .joined()
            insertText(line, replacementRange: range)
            return false
        }
        return super.shouldChangeText(in: range, replacementString: string)
    }

    override func didChangeText()
    {
        super.didChangeText()
        invalidateIntrinsicContentSize()
        if !updating {
            changed(pieces())
        }
    }

    override func doCommand(by selector: Selector)
    {
        switch selector {
        case #selector(NSResponder.insertNewline(_:)),
            #selector(NSResponder.insertLineBreak(_:)),
            #selector(NSResponder.insertNewlineIgnoringFieldEditor(_:)):
            submit()
        case #selector(NSResponder.insertTab(_:)):
            window?.selectNextKeyView(nil)
        case #selector(NSResponder.insertBacktab(_:)):
            window?.selectPreviousKeyView(nil)
        case #selector(NSResponder.cancelOperation(_:)):
            cancel()
        default:
            super.doCommand(by: selector)
        }
    }

    func insert_plain(_ text: String?)
    {
        guard let text, isEditable else {
            return
        }
        let line: String = text.components(separatedBy: TOKEN_TEXT_VIEW.NEWLINES).joined()
        replace(selectedRange(), with: NSAttributedString(string: line,
            attributes: TOKEN_TEXT_VIEW.text_attributes()))
    }

    override func paste(_ sender: Any?)
    {
        insert_plain(NSPasteboard.general.string(forType: .string))
    }

    override func pasteAsPlainText(_ sender: Any?)
    {
        paste(sender)
    }

    override func pasteAsRichText(_ sender: Any?)
    {
        paste(sender)
    }

    override func readSelection(from board: NSPasteboard, type: NSPasteboard.PasteboardType)
        -> Bool
    {
        guard let text = board.string(forType: .string) else {
            return false
        }
        insert_plain(text)
        return true
    }

    override func writeSelection(to board: NSPasteboard, type: NSPasteboard.PasteboardType)
        -> Bool
    {
        let text: String = (owned_storage.string as NSString).substring(with: selectedRange())
            .filter { $0 != TOKEN_TEXT_VIEW.ATTACHMENT_CHARACTER }
        board.clearContents()
        return board.setString(text, forType: .string)
    }

    override func becomeFirstResponder() -> Bool
    {
        let accepted: Bool = super.becomeFirstResponder()
        if accepted {
            focus_changed(true)
        }
        return accepted
    }

    override func resignFirstResponder() -> Bool
    {
        let accepted: Bool = super.resignFirstResponder()
        if accepted {
            focus_changed(false)
        }
        return accepted
    }

    private func replace(_ range: NSRange, with text: NSAttributedString)
    {
        if !shouldChangeText(in: range, replacementString: text.string) {
            return
        }
        owned_storage.replaceCharacters(in: range, with: text)
        didChangeText()
        setSelectedRange(NSRange(location: range.location + text.length, length: 0))
    }

    private func setup()
    {
        isRichText = true
        importsGraphics = false
        allowsUndo = true
        isEditable = true
        isSelectable = true
        drawsBackground = false
        focusRingType = .none
        isVerticallyResizable = false
        isHorizontallyResizable = false
        textContainerInset = TOKEN_TEXT_VIEW.INSET
        font = TOKEN_TEXT_VIEW.FONT
        textColor = .labelColor
        insertionPointColor = .labelColor
        smartInsertDeleteEnabled = false
        isAutomaticQuoteSubstitutionEnabled = false
        isAutomaticDashSubstitutionEnabled = false
        isAutomaticTextReplacementEnabled = false
        isAutomaticSpellingCorrectionEnabled = false
        isContinuousSpellCheckingEnabled = false
        isGrammarCheckingEnabled = false
        isAutomaticDataDetectionEnabled = false
        isAutomaticLinkDetectionEnabled = false
        isAutomaticTextCompletionEnabled = false
        usesFindBar = false
        usesFontPanel = false
        usesRuler = false
    }

    private static func chip(_ token: TOKEN_KIND) -> NSAttributedString
    {
        let attachment: NSTextAttachment = NSTextAttachment()
        attachment.attachmentCell = TOKEN_CHIP_CELL(token: token)
        let text: NSMutableAttributedString = NSMutableAttributedString(attachment: attachment)
        text.addAttributes(text_attributes(), range: NSRange(location: 0, length: text.length))
        return text
    }

    private static func text_attributes() -> [NSAttributedString.Key: Any]
    {
        return [.font: FONT, .foregroundColor: NSColor.labelColor]
    }
}
