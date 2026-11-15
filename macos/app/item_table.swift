import AppKit
import SwiftUI

import limenamer_view_model

struct ITEM_TABLE: NSViewRepresentable
{
    let main: MAIN_VIEW_MODEL
    let rows: [ITEM_ROW]
    let hidden: Set<ITEM_COLUMN>
    let checked: Set<UInt64>

    func makeCoordinator() -> ITEM_TABLE_CONTROLLER
    {
        return ITEM_TABLE_CONTROLLER(main: main)
    }

    func makeNSView(context: Context) -> NSScrollView
    {
        let controller: ITEM_TABLE_CONTROLLER = context.coordinator
        controller.update(rows: rows, hidden: hidden, checked: checked)
        return controller.scroll_view
    }

    func updateNSView(_ view: NSScrollView, context: Context)
    {
        context.coordinator.update(rows: rows, hidden: hidden, checked: checked)
    }
}
