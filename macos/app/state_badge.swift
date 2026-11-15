import SwiftUI

struct STATE_BADGE: View
{
    let text: String
    let existing: Bool

    var body: some View
    {
        Text(text)
            .font(.system(size: 11.5, weight: existing ? .semibold : .regular))
            .foregroundStyle(existing ? Color(nsColor: PALETTE.ACCENT_TEXT) : .secondary)
            .padding(.horizontal, 8)
            .frame(height: 20)
            .background(Capsule().fill(Color(nsColor: existing ? PALETTE.ACCENT_SOFT
                : PALETTE.NEUTRAL_BACK)))
    }
}
