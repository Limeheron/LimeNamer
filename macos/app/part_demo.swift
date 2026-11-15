import SwiftUI

import limenamer_view_model

struct PART_DEMO: View
{
    private static let FONT: Font = .system(size: 12, design: .monospaced)
    private static let MARK_FONT: Font = .system(size: 12, weight: .medium, design: .monospaced)

    let part: PART

    var body: some View
    {
        let name: String = STRINGS.sample_name
        let ext: String = STRINGS.sample_ext
        var text: AttributedString
        switch part {
        case .name:
            text = PART_DEMO.marked(name)
            text.append(AttributedString("." + ext))
        case .ext:
            text = AttributedString(name + ".")
            text.append(PART_DEMO.marked(ext))
        case .full:
            text = PART_DEMO.marked(name + "." + ext)
        }
        return Text(text)
            .font(PART_DEMO.FONT)
            .foregroundStyle(.secondary)
    }

    private static func marked(_ value: String) -> AttributedString
    {
        var text: AttributedString = AttributedString(value)
        text.font = MARK_FONT
        text.foregroundColor = Color(nsColor: PALETTE.ACCENT_TEXT)
        text.backgroundColor = Color(nsColor: PALETTE.ACCENT_SOFT)
        return text
    }
}
