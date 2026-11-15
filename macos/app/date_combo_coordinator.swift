import AppKit

@MainActor
final class DATE_COMBO_COORDINATOR: NSObject, NSComboBoxDelegate
{
    private static let LIMIT: Int = 64

    var changed: (String) -> Void = { _ in }
    var submit: () -> Void = {}
    var cancel: () -> Void = {}

    func controlTextDidChange(_ notification: Notification)
    {
        guard let box = notification.object as? NSComboBox else {
            return
        }
        let text: String = box.stringValue
        if text.count > DATE_COMBO_COORDINATOR.LIMIT {
            box.stringValue = String(text.prefix(DATE_COMBO_COORDINATOR.LIMIT))
        }
        changed(box.stringValue)
    }

    func comboBoxSelectionDidChange(_ notification: Notification)
    {
        guard let box = notification.object as? NSComboBox,
            let text = box.objectValueOfSelectedItem as? String else {
            return
        }
        changed(text)
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy selector: Selector)
        -> Bool
    {
        switch selector {
        case #selector(NSResponder.insertNewline(_:)):
            submit()
            return true
        case #selector(NSResponder.cancelOperation(_:)):
            cancel()
            return true
        default:
            return false
        }
    }
}
