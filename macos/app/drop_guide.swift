import AppKit
import SwiftUI

import limenamer_view_model

struct DROP_GUIDE: View
{
    private static let RADIUS: CGFloat = 10
    private static let INSET: CGFloat = 8

    var body: some View
    {
        let shape: RoundedRectangle = RoundedRectangle(cornerRadius: DROP_GUIDE.RADIUS)
        ContentUnavailableView {
            Label {
                Text(STRINGS.drop_title)
            } icon: {
                Image(systemName: "square.and.arrow.down")
                    .foregroundStyle(Color(nsColor: PALETTE.ACCENT_TEXT))
            }
        } description: {
            Text(STRINGS.drop_text)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(shape.fill(Color(nsColor: .controlBackgroundColor)))
        .overlay(shape.strokeBorder(Color(nsColor: PALETTE.ACCENT_TEXT), lineWidth: 2))
        .padding(DROP_GUIDE.INSET)
        .allowsHitTesting(false)
    }
}
