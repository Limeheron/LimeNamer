import AppKit

import limenamer_view_model

@MainActor
enum FILE_PICKER
{
    static func add(_ main: MAIN_VIEW_MODEL, folders: Bool, mode: ADD_MODE)
    {
        Task {
            let paths: [String] = await FILE_PICKER.choose(folders: folders)
            main.add_paths(paths, mode: mode)
        }
    }

    static func choose(folders: Bool) async -> [String]
    {
        let panel: NSOpenPanel = NSOpenPanel()
        panel.canChooseFiles = !folders
        panel.canChooseDirectories = folders
        panel.allowsMultipleSelection = true
        let response: NSApplication.ModalResponse
        if let window = NSApp.keyWindow {
            response = await panel.beginSheetModal(for: window)
        } else {
            response = panel.runModal()
        }
        if response != .OK {
            return []
        }
        return panel.urls.map { FILE_PICKER.path($0) }
    }

    private static func path(_ url: URL) -> String
    {
        var path: String = url.path(percentEncoded: false)
        while path.count > 1 && path.hasSuffix("/") {
            path.removeLast()
        }
        return path
    }
}
