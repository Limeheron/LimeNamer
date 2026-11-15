import SwiftUI

struct CARD_BOX: View
{
    private static let RADIUS: CGFloat = 10

    var hovering: Bool = false

    var body: some View
    {
        let shape: RoundedRectangle = RoundedRectangle(cornerRadius: CARD_BOX.RADIUS)
        shape
            .fill(Color(nsColor: hovering ? PALETTE.CARD_HOVER_BACK : PALETTE.CARD_BACK))
            .shadow(color: .black.opacity(hovering ? 0.08 : 0.04), radius: hovering ? 1.5 : 0.5,
                y: 1)
            .overlay(shape.strokeBorder(Color(nsColor: hovering ? PALETTE.CARD_HOVER_LINE
                : PALETTE.CARD_LINE), lineWidth: 1))
    }
}
