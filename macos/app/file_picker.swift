import AppKit

@MainActor
enum FILE_PICKER
{
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
