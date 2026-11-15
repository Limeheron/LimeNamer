import AppKit
import SwiftUI

import limenamer_view_model

struct SAMPLE_TEXT: View
{
    let pieces: [SAMPLE_PIECE]
    var size: CGFloat = 12

    var body: some View
    {
        HStack(spacing: 0) {
            ForEach(Array(pieces.enumerated()), id: \.offset) { _, piece in
                if piece.style == .caret {
                    RoundedRectangle(cornerRadius: 1)
                        .fill(Color(nsColor: PALETTE.ACCENT_TEXT))
                        .frame(width: 2, height: size + 1)
                        .padding(.horizontal, 1)
                } else {
                    SAMPLE_TEXT.text(piece, size: size)
                }
            }
        }
        .lineLimit(1)
        .fixedSize()
    }

    private static func text(_ piece: SAMPLE_PIECE, size: CGFloat) -> Text
    {
        let plain: Text = Text(piece.text).font(.system(size: size, design: .monospaced))
        let bold: Text = Text(piece.text).font(.system(size: size, weight: .bold,
            design: .monospaced))
        let accent: Color = Color(nsColor: PALETTE.ACCENT_TEXT)
        switch piece.style {
        case .plain, .caret:
            return plain
        case .mark:
            return bold.foregroundStyle(accent)
        case .removed:
            return bold.foregroundStyle(Color(nsColor: PALETTE.ERROR_TEXT))
        case .anchor:
            return bold.underline(true, pattern: .dot)
        case .mark_anchor:
            return bold.foregroundStyle(accent).underline(true, pattern: .dot)
        }
    }
}
