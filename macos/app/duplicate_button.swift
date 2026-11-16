import SwiftUI

import limenamer_view_model

struct DUPLICATE_BUTTON: View
{
    let main: MAIN_VIEW_MODEL

    @State private var showing: Bool = false

    var body: some View
    {
        Toggle(isOn: Binding(get: {
            return main.duplicate_enabled
        }, set: { _ in
            showing = true
        })) {
            Label(STRINGS.toolbar_duplicate, systemImage: "plus.square.on.square")
        }
        .toggleStyle(.button)
        .help(STRINGS.toolbar_duplicate)
        .disabled(!main.can_edit)
        .popover(isPresented: $showing, arrowEdge: .bottom) {
            DUPLICATE_POPOVER(main: main)
        }
    }
}
