import AppKit

final class WINDOW_SETUP_VIEW: NSView
{
    override func viewDidMoveToWindow()
    {
        super.viewDidMoveToWindow()
        window?.isRestorable = false
    }
}
