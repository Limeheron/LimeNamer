import SwiftUI

struct WINDOW_SETUP: NSViewRepresentable
{
    let state: WINDOW_STATE

    func makeNSView(context: Context) -> WINDOW_SETUP_VIEW
    {
        let view: WINDOW_SETUP_VIEW = WINDOW_SETUP_VIEW()
        view.state = state
        return view
    }

    func updateNSView(_ view: WINDOW_SETUP_VIEW, context: Context)
    {
    }
}
