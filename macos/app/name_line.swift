import AppKit
import SwiftUI

struct NAME_LINE: View
{
    private static let FONT: Font = .system(size: 13, design: .monospaced)
    private static let MARK_FONT: Font = .system(size: 13, weight: .bold, design: .monospaced)

    let parts: (head: String, mark: String, tail: String)
    let mark: NSColor

    var body: some View
    {
        var text: AttributedString = AttributedString(parts.head)
        var marked: AttributedString = AttributedString(parts.mark)
        marked.foregroundColor = Color(nsColor: mark)
        marked.font = NAME_LINE.MARK_FONT
        text.append(marked)
        text.append(AttributedString(parts.tail))
        return Text(text)
            .font(NAME_LINE.FONT)
            .lineLimit(1)
            .truncationMode(.tail)
            .help(parts.head + parts.mark + parts.tail)
    }
}
