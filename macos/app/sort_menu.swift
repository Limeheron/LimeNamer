import SwiftUI

import limenamer_view_model

struct SORT_MENU: View
{
    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        let custom: Bool = main.sort_key == .custom
        ForEach(MAIN_VIEW_MODEL.SORT_CHOICES, id: \.value) { choice in
            Toggle(choice.label, isOn: Binding(get: {
                return main.sort_key == choice.value
            }, set: { _ in
                main.sort(choice.value, descending: main.sort_descending && !custom)
            }))
        }
        Toggle(STRINGS.sort_custom, isOn: .constant(custom))
            .disabled(true)
        Divider()
        Toggle(STRINGS.sort_ascending, isOn: Binding(get: {
            return !custom && !main.sort_descending
        }, set: { _ in
            main.sort(main.sort_key, descending: false)
        }))
        .disabled(custom)
        Toggle(STRINGS.sort_descending, isOn: Binding(get: {
            return !custom && main.sort_descending
        }, set: { _ in
            main.sort(main.sort_key, descending: true)
        }))
        .disabled(custom)
    }
}
