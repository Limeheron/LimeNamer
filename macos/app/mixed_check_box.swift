import AppKit
import SwiftUI

struct MIXED_CHECK_BOX: NSViewRepresentable
{
    let state: Bool?
    let tip: String
    let clicked: () -> Void

    func makeCoordinator() -> ACTION_TARGET
    {
        return ACTION_TARGET()
    }

    func makeNSView(context: Context) -> NSButton
    {
        let button: NSButton = NSButton(checkboxWithTitle: "", target: context.coordinator,
            action: #selector(ACTION_TARGET.fire(_:)))
        button.allowsMixedState = true
        button.toolTip = tip
        updateNSView(button, context: context)
        return button
    }

    func updateNSView(_ button: NSButton, context: Context)
    {
        let value: NSControl.StateValue = MIXED_CHECK_BOX.value(state)
        let clicked: () -> Void = clicked
        button.state = value
        button.isEnabled = context.environment.isEnabled
        context.coordinator.action = { [weak button] in
            button?.state = value
            clicked()
        }
    }

    private static func value(_ state: Bool?) -> NSControl.StateValue
    {
        switch state {
        case .some(true):
            return .on
        case .some(false):
            return .off
        case .none:
            return .mixed
        }
    }
}
