import SwiftUI

import limenamer_view_model

struct POSITION_TILE_VIEW: View
{
    private static let RADIUS: CGFloat = 9

    let choice: POSITION_CHOICE
    let selected: Bool
    let action: () -> Void

    @State private var hovering: Bool = false

    var body: some View
    {
        Button(action: action) {
            VStack(alignment: .leading, spacing: 4) {
                Text(choice.label)
                    .font(.system(size: 12, weight: .semibold))
                    .fixedSize(horizontal: false, vertical: true)
                SAMPLE_TEXT(pieces: choice.tile)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
            .padding(.vertical, 8)
            .padding(.horizontal, 10)
            .background(background)
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .accessibilityAddTraits(selected ? .isSelected : [])
        .onHover { inside in
            hovering = inside
        }
    }

    private var background: some View
    {
        let shape: RoundedRectangle = RoundedRectangle(cornerRadius: POSITION_TILE_VIEW.RADIUS)
        let line: NSColor = selected ? PALETTE.ACCENT_TEXT
            : hovering ? PALETTE.CARD_HOVER_LINE : PALETTE.CARD_LINE
        return shape
            .fill(Color(nsColor: hovering && !selected ? PALETTE.CARD_HOVER_BACK
                : PALETTE.CARD_BACK))
            .overlay(shape.strokeBorder(Color(nsColor: line), lineWidth: selected ? 1.5 : 1))
            .overlay {
                if selected {
                    RoundedRectangle(cornerRadius: POSITION_TILE_VIEW.RADIUS + 1.5)
                        .stroke(Color(nsColor: PALETTE.SELECT_RING), lineWidth: 3)
                        .padding(-1.5)
                }
            }
    }
}
