import AppKit

import limenamer_view_model

@MainActor
final class ALERT_DIALOGS: DIALOG_SERVICE
{
    func ask(_ content: DIALOG_CONTENT) async -> Bool
    {
        let alert: NSAlert = NSAlert()
        alert.alertStyle = content.kind == .report ? .informational : .warning
        alert.messageText = content.title
        alert.informativeText = ALERT_DIALOGS.details(content)
        let primary: NSButton = alert.addButton(withTitle: content.primary)
        if !content.secondary.isEmpty {
            let secondary: NSButton = alert.addButton(withTitle: content.secondary)
            if content.kind == .confirm {
                secondary.keyEquivalent = "\u{1b}"
            } else {
                primary.keyEquivalent = ""
            }
        }
        let response: NSApplication.ModalResponse
        if let window = NSApp.keyWindow {
            response = await alert.beginSheetModal(for: window)
        } else {
            response = alert.runModal()
        }
        return response == .alertFirstButtonReturn
    }

    private static func details(_ content: DIALOG_CONTENT) -> String
    {
        var parts: [String] = [content.message]
        var lines: [String] = []
        for line in content.lines {
            var text: String = line.index > 0 ? "\(line.index). \(line.text)" : line.text
            if !line.reason.isEmpty {
                text += "\n    \(line.reason)"
            }
            lines.append(text)
        }
        if !lines.isEmpty {
            parts.append(lines.joined(separator: "\n"))
        }
        if !content.note.isEmpty {
            parts.append(content.note)
        }
        return parts.joined(separator: "\n\n")
    }
}
