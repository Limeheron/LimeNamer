import SwiftUI

struct PANEL_HEADER<Content: View>: View
{
    static var HEIGHT: CGFloat
    {
        return 44
    }

    let leading: CGFloat
    @ViewBuilder let content: Content

    var body: some View
    {
        HStack(spacing: 10) {
            content
        }
        .padding(.leading, leading)
        .padding(.trailing, 10)
        .frame(height: PANEL_HEADER.HEIGHT)
    }
}
