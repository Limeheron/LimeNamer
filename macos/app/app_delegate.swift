import AppKit

import limenamer_view_model

@MainActor
final class APP_DELEGATE: NSObject, NSApplicationDelegate
{
    weak var main: MAIN_VIEW_MODEL?

    func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply
    {
        guard let main else {
            return .terminateNow
        }
        return main.allow_close() ? .terminateNow : .terminateCancel
    }
}
