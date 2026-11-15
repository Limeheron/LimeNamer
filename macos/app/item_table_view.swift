import AppKit

final class ITEM_TABLE_VIEW: NSTableView
{
    private static let RETURN_KEYS: Set<String> = ["\r", "\u{3}"]

    var return_pressed: (() -> Void)? = nil

    override func keyDown(with event: NSEvent)
    {
        let modifiers: NSEvent.ModifierFlags = event.modifierFlags
            .intersection([.command, .option, .control, .shift])
        if let return_pressed, modifiers.isEmpty,
            ITEM_TABLE_VIEW.RETURN_KEYS.contains(event.charactersIgnoringModifiers ?? "") {
            return_pressed()
            return
        }
        super.keyDown(with: event)
    }
}
