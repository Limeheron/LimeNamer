import SwiftUI

struct FIELD_FRAME<Content: View>: View
{
    static var HEIGHT: CGFloat
    {
        return 32
    }

    private static var RADIUS: CGFloat
    {
        return 8
    }

    private static var RING: CGFloat
    {
        return 3
    }

    let focused: Bool
    let problem: String
    @ViewBuilder let content: Content

    @Environment(\.isEnabled) private var enabled: Bool

    var body: some View
    {
        let shape: RoundedRectangle = RoundedRectangle(cornerRadius: FIELD_FRAME.RADIUS)
        let bad: Bool = !problem.isEmpty
        VStack(alignment: .leading, spacing: 4) {
            content
                .frame(minHeight: FIELD_FRAME.HEIGHT)
                .background(shape.fill(Color(nsColor: PALETTE.FIELD_BACK)))
                .overlay(shape.strokeBorder(Color(nsColor: line_color(bad)), lineWidth: 1))
                .overlay {
                    if bad || focused {
                        RoundedRectangle(cornerRadius: FIELD_FRAME.RADIUS + FIELD_FRAME.RING / 2)
                            .stroke(Color(nsColor: bad ? PALETTE.ERROR_RING : PALETTE.FOCUS_RING),
                                lineWidth: FIELD_FRAME.RING)
                            .padding(-FIELD_FRAME.RING / 2)
                            .allowsHitTesting(false)
                    }
                }
                .opacity(enabled ? 1 : 0.5)
            if bad {
                Text(problem)
                    .font(.system(size: 11.5))
                    .foregroundStyle(Color(nsColor: PALETTE.ERROR_TEXT))
                    .fixedSize(horizontal: false, vertical: true)
            }
        }
    }

    private func line_color(_ bad: Bool) -> NSColor
    {
        if bad {
            return PALETTE.ERROR_TEXT
        }
        return focused ? PALETTE.ACCENT_TEXT : PALETTE.FIELD_LINE
    }
}
