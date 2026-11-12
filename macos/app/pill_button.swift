import SwiftUI

import limenamer_view_model

struct PILL_BUTTON: View
{
    let text: String
    let status: STATUS
    let active: Bool
    let action: () -> Void

    var body: some View
    {
        Button(action: action) {
            Label(text, systemImage: STATUS_LOOK.symbol(status))
                .labelStyle(.titleAndIcon)
                .font(.callout)
                .padding(.horizontal, 8)
                .frame(height: 26)
                .background(Capsule().fill(active
                    ? Color(nsColor: STATUS_LOOK.back(status)) : Color.clear))
                .contentShape(Capsule())
        }
        .buttonStyle(.plain)
        .foregroundStyle(Color(nsColor: STATUS_LOOK.text(status)))
        .accessibilityAddTraits(active ? .isSelected : [])
    }
}
