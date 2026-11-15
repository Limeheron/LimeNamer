import AppKit
import SwiftUI

import limenamer_view_model

struct DATE_COMBO: NSViewRepresentable
{
    static let FORMATS: [String] = MOD_EDITOR_VIEW_MODEL.DATE_FORMAT_CHOICES
        .map { $0.value }
        .filter { !$0.isEmpty }
    private static let FONT: NSFont = NSFont.monospacedSystemFont(ofSize: 13, weight: .regular)

    let text: String
    let handle: VIEW_HANDLE
    let changed: (String) -> Void
    let submit: () -> Void
    let cancel: () -> Void

    func makeCoordinator() -> DATE_COMBO_COORDINATOR
    {
        return DATE_COMBO_COORDINATOR()
    }

    func makeNSView(context: Context) -> NSComboBox
    {
        let box: NSComboBox = NSComboBox()
        box.isEditable = true
        box.completes = false
        box.usesDataSource = false
        box.font = DATE_COMBO.FONT
        box.addItems(withObjectValues: DATE_COMBO.FORMATS)
        box.numberOfVisibleItems = DATE_COMBO.FORMATS.count
        box.delegate = context.coordinator
        handle.view = box
        updateNSView(box, context: context)
        return box
    }

    func updateNSView(_ box: NSComboBox, context: Context)
    {
        let coordinator: DATE_COMBO_COORDINATOR = context.coordinator
        coordinator.changed = changed
        coordinator.submit = submit
        coordinator.cancel = cancel
        box.isEnabled = context.environment.isEnabled
        if box.stringValue != text {
            box.stringValue = text
        }
    }
}
