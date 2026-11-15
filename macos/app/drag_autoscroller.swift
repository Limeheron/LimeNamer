import AppKit
import QuartzCore

@MainActor
final class DRAG_AUTOSCROLLER: NSObject
{
    private static let EDGE: CGFloat = 84
    private static let MIN_SPEED: CGFloat = 30
    private static let MAX_SPEED: CGFloat = 2400
    private static let MAX_FRAME_TIME: CFTimeInterval = 0.05

    weak var scroll_view: NSScrollView? = nil
    var scrolled: (() -> Void)? = nil
    private var link: CADisplayLink? = nil
    private var carry: CGFloat = 0
    private var last_time: CFTimeInterval = 0

    func start()
    {
        guard link == nil, let scroll_view else {
            return
        }
        let created: CADisplayLink = scroll_view.displayLink(target: self,
            selector: #selector(tick(_:)))
        created.add(to: .main, forMode: .common)
        link = created
        carry = 0
        last_time = 0
    }

    func stop()
    {
        link?.invalidate()
        link = nil
    }

    @objc private func tick(_ sender: CADisplayLink)
    {
        let elapsed: CFTimeInterval = last_time > 0
            ? min(sender.timestamp - last_time, DRAG_AUTOSCROLLER.MAX_FRAME_TIME) : 0
        last_time = sender.timestamp
        guard let scroll_view, let window = scroll_view.window else {
            return
        }
        let speed: CGFloat = speed_at(window.mouseLocationOutsideOfEventStream)
        if speed == 0 || (carry != 0 && (carry > 0) != (speed > 0)) {
            carry = 0
        }
        carry += speed * CGFloat(elapsed)
        let step: CGFloat = carry.rounded(.towardZero)
        if step == 0 {
            return
        }
        carry -= step
        let clip: NSClipView = scroll_view.contentView
        var bounds: NSRect = clip.bounds
        bounds.origin.y += step
        let origin: NSPoint = clip.constrainBoundsRect(bounds).origin
        if origin == clip.bounds.origin {
            return
        }
        clip.scroll(to: origin)
        scroll_view.reflectScrolledClipView(clip)
        scrolled?()
    }

    private func speed_at(_ window_point: NSPoint) -> CGFloat
    {
        guard let scroll_view, scroll_view.contentView.isFlipped else {
            return 0
        }
        let x: CGFloat = scroll_view.convert(window_point, from: nil).x
        if x < scroll_view.bounds.minX || x > scroll_view.bounds.maxX {
            return 0
        }
        let clip: NSClipView = scroll_view.contentView
        let edge: CGFloat = DRAG_AUTOSCROLLER.EDGE
        let y: CGFloat = clip.convert(window_point, from: nil).y
        let top: CGFloat = clip.bounds.minY + clip.contentInsets.top + edge
        let bottom: CGFloat = clip.bounds.maxY - clip.contentInsets.bottom - edge
        if y < top {
            return -DRAG_AUTOSCROLLER.speed(depth: (top - y) / edge)
        }
        if y > bottom {
            return DRAG_AUTOSCROLLER.speed(depth: (y - bottom) / edge)
        }
        return 0
    }

    private static func speed(depth: CGFloat) -> CGFloat
    {
        let clamped: CGFloat = min(max(depth, 0), 1)
        return MIN_SPEED * pow(MAX_SPEED / MIN_SPEED, clamped)
    }
}
