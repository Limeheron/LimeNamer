import SwiftUI

struct TOKEN_SETTINGS_BOX: ViewModifier
{
    func body(content: Content) -> some View
    {
        content
            .font(.system(size: 12.5))
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(.vertical, 8)
            .padding(.horizontal, 10)
            .background(CARD_BOX())
    }
}
