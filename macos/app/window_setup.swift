import SwiftUI

struct WINDOW_SETUP: NSViewRepresentable
{
    func makeNSView(context: Context) -> WINDOW_SETUP_VIEW
    {
        return WINDOW_SETUP_VIEW()
    }

    func updateNSView(_ view: WINDOW_SETUP_VIEW, context: Context)
    {
    }
}
