import AppKit
import SwiftUI

struct PANEL_DIVIDER: View
{
    private static let HIT_WIDTH: CGFloat = 8

    @Binding var width: CGFloat
    let current: CGFloat
    let limit: ClosedRange<CGFloat>

    @State private var start_width: CGFloat? = nil
    @State private var hovering: Bool = false

    var body: some View
    {
        Divider()
            .overlay {
                Color.clear
                    .frame(width: PANEL_DIVIDER.HIT_WIDTH)
                    .contentShape(Rectangle())
                    .onHover { inside in
                        set_hovering(inside)
                    }
                    .onDisappear {
                        set_hovering(false)
                    }
                    .gesture(drag)
            }
    }

    private var drag: some Gesture
    {
        DragGesture(minimumDistance: 1, coordinateSpace: .global)
            .onChanged { value in
                let start: CGFloat = start_width ?? current
                start_width = start
                let proposed: CGFloat = start - value.translation.width
                width = min(max(proposed, limit.lowerBound), limit.upperBound)
            }
            .onEnded { _ in
                start_width = nil
            }
    }

    private func set_hovering(_ inside: Bool)
    {
        if inside == hovering {
            return
        }
        hovering = inside
        if inside {
            NSCursor.resizeLeftRight.push()
        } else {
            NSCursor.pop()
        }
    }
}
