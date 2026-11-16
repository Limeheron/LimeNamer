import SwiftUI

import limenamer_view_model

struct DUPLICATE_BUTTON: View
{
    let main: MAIN_VIEW_MODEL
    @Bindable var state: WINDOW_STATE

    var body: some View
    {
        Toggle(isOn: Binding(get: {
            return main.duplicate_enabled
        }, set: { _ in
            state.duplicate_shown = true
        })) {
            Label(STRINGS.toolbar_duplicate, systemImage: "plus.square.on.square")
        }
        .toggleStyle(.button)
        .help(STRINGS.toolbar_duplicate)
        .disabled(!main.can_edit)
        .popover(isPresented: $state.duplicate_shown, arrowEdge: .bottom) {
            DUPLICATE_POPOVER(main: main)
        }
    }
}
