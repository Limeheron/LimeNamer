import AppKit

final class WINDOW_SETUP_VIEW: NSView
{
    weak var state: WINDOW_STATE? = nil

    override func viewDidMoveToWindow()
    {
        super.viewDidMoveToWindow()
        guard let window else {
            return
        }
        window.isRestorable = false
        state?.attach(window)
    }
}
