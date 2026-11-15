import AppKit

@MainActor
final class VIEW_HANDLE
{
    weak var view: NSView? = nil

    func focus()
    {
        guard let view, let window = view.window else {
            return
        }
        view.scrollToVisible(view.bounds)
        window.makeFirstResponder(view)
    }
}
