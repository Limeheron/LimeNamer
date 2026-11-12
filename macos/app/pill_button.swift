import SwiftUI

struct PILL_BUTTON: View
{
    let text: String
    let symbol: String
    let tint: Color
    let back: Color
    let active: Bool
    let action: () -> Void

    var body: some View
    {
        Button(action: action) {
            Label(text, systemImage: symbol)
                .labelStyle(.titleAndIcon)
                .font(.callout)
                .padding(.horizontal, 8)
                .frame(height: 26)
                .background(Capsule().fill(active ? back : Color.clear))
                .contentShape(Capsule())
        }
        .buttonStyle(.plain)
        .foregroundStyle(tint)
        .accessibilityAddTraits(active ? .isSelected : [])
    }
}
