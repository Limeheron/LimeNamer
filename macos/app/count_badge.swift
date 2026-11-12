import SwiftUI

struct COUNT_BADGE: View
{
    let count: Int

    var body: some View
    {
        Text(String(count))
            .font(.system(size: 10.5, weight: .bold))
            .monospacedDigit()
            .foregroundStyle(PALETTE.ACCENT_ON_FILL)
            .padding(.horizontal, 4)
            .frame(minWidth: 16, minHeight: 16)
            .background(Capsule().fill(PALETTE.ACCENT_FILL))
    }
}
