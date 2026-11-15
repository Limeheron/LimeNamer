import AppKit
import SwiftUI

import limenamer_view_model

struct MOD_CARD_TABLE: NSViewRepresentable
{
    let main: MAIN_VIEW_MODEL
    let cards: [MOD_CARD]

    func makeCoordinator() -> MOD_CARD_CONTROLLER
    {
        return MOD_CARD_CONTROLLER(main: main)
    }

    func makeNSView(context: Context) -> NSScrollView
    {
        context.coordinator.update(cards, enabled: context.environment.isEnabled)
        return context.coordinator.scroll_view
    }

    func updateNSView(_ view: NSScrollView, context: Context)
    {
        context.coordinator.update(cards, enabled: context.environment.isEnabled)
    }
}
