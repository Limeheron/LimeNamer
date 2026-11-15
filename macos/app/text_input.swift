import SwiftUI

import limenamer_view_model

struct TEXT_INPUT: View
{
    static let FONT: Font = .system(size: 14, design: .monospaced)

    let text: Binding<String>
    let problem: String
    let field: MOD_FIELD
    let focus: FocusState<MOD_FIELD?>.Binding

    var body: some View
    {
        FIELD_FRAME(focused: focus.wrappedValue == field, problem: problem) {
            TextField("", text: text)
                .textFieldStyle(.plain)
                .font(TEXT_INPUT.FONT)
                .autocorrectionDisabled()
                .padding(.horizontal, 10)
                .focused(focus, equals: field)
        }
        .id(field)
    }
}
