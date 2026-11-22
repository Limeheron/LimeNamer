import AppKit
import SwiftUI

import limenamer_view_model

@MainActor
enum ABOUT_PANEL
{
    private static var window: NSWindow?

    static func show(version: String)
    {
        let target: NSWindow = window ?? make(version: version)
        window = target
        target.makeKeyAndOrderFront(nil)
        NSApp.activate()
    }

    private static func make(version: String) -> NSWindow
    {
        let result: NSWindow = NSWindow(contentViewController:
            NSHostingController(rootView: ABOUT_VIEW(version: version)))
        result.styleMask = [.titled, .closable]
        result.titlebarAppearsTransparent = true
        result.titleVisibility = .hidden
        result.title = STRINGS.info_title
        result.isReleasedWhenClosed = false
        result.center()
        return result
    }
}
