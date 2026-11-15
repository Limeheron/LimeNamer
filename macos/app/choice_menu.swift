import SwiftUI

import limenamer_view_model

struct CHOICE_MENU<T: Hashable & Sendable>: View
{
    let choices: [CHOICE<T>]
    let selection: Binding<T>

    var body: some View
    {
        Picker("", selection: selection) {
            ForEach(choices, id: \.label) { choice in
                Text(choice.label)
                    .tag(choice.value)
            }
        }
        .pickerStyle(.menu)
        .labelsHidden()
        .frame(maxWidth: .infinity)
    }
}
