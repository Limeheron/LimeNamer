import SwiftUI

import limenamer_view_model

struct MOD_VALUE_FIELDS: View
{
    private static let TOKENS: [TOKEN_KIND] = [.number, .original_name, .modified_date,
        .created_date, .size, .parent_folder]
    private static let NUMBER: ClosedRange<Int> = -1_000_000_000...1_000_000_000

    let main: MAIN_VIEW_MODEL
    @Bindable var editor: MOD_EDITOR_VIEW_MODEL
    let handles: (token: VIEW_HANDLE, modified: VIEW_HANDLE, created: VIEW_HANDLE)

    @State private var token_focused: Bool = false

    var body: some View
    {
        FORM_FIELD(label: editor.value_label) {
            FIELD_FRAME(focused: token_focused, problem: editor.problem(.value)) {
                TOKEN_FIELD(pieces: editor.value, handle: handles.token, changed: { pieces in
                    editor.value = pieces
                }, focus_changed: { focused in
                    token_focused = focused
                }, submit: {
                    MOD_ACTIONS.apply(main)
                }, cancel: {
                    MOD_ACTIONS.cancel(main)
                })
            }
            .id(MOD_FIELD.value)
            FLOW_LAYOUT(spacing: 6) {
                ForEach(MOD_VALUE_FIELDS.TOKENS, id: \.self) { token in
                    Button {
                        (handles.token.view as? TOKEN_TEXT_VIEW)?.insert_token(token)
                    } label: {
                        Label(TOKEN_CHIP_CELL.name(token), systemImage: "plus")
                            .font(.system(size: 12))
                    }
                    .buttonStyle(.bordered)
                    .buttonBorderShape(.capsule)
                    .controlSize(.small)
                }
            }
            if editor.uses_number {
                number_settings
            }
            if editor.uses_modified_date {
                date_settings(STRINGS.modified_date_format_label,
                    choice: $editor.modified_date_choice, custom: $editor.modified_date_custom,
                    field: .modified_date_format, handle: handles.modified)
            }
            if editor.uses_created_date {
                date_settings(STRINGS.created_date_format_label,
                    choice: $editor.created_date_choice, custom: $editor.created_date_custom,
                    field: .created_date_format, handle: handles.created)
            }
            if editor.uses_size {
                HStack(spacing: 10) {
                    Text(STRINGS.size_unit_label)
                    CHOICE_MENU(choices: MOD_EDITOR_VIEW_MODEL.SIZE_UNIT_CHOICES,
                        selection: $editor.size_unit)
                }
                .modifier(TOKEN_SETTINGS_BOX())
            }
        }
    }

    private var number_settings: some View
    {
        HStack(alignment: .top, spacing: 8) {
            number_setting(STRINGS.number_start, Int(editor.number_start),
                MOD_VALUE_FIELDS.NUMBER) { value in
                editor.number_start = Int32(value)
            }
            number_setting(STRINGS.number_step, Int(editor.number_step),
                MOD_VALUE_FIELDS.NUMBER) { value in
                editor.number_step = Int32(value)
            }
            number_setting(STRINGS.number_digits, Int(editor.number_digits),
                MOD_POSITION_FIELDS.DIGITS) { value in
                editor.number_digits = UInt32(value)
            }
        }
        .modifier(TOKEN_SETTINGS_BOX())
    }

    private func number_setting(_ label: String, _ value: Int, _ range: ClosedRange<Int>,
        _ changed: @escaping (Int) -> Void) -> some View
    {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
            NUMBER_INPUT(value: value, range: range, fills: true, changed: changed)
        }
    }

    private func date_settings(_ label: String, choice: Binding<String>, custom: Binding<String>,
        field: MOD_FIELD, handle: VIEW_HANDLE) -> some View
    {
        let problem: String = editor.problem(field)
        let text: String = choice.wrappedValue.isEmpty ? custom.wrappedValue : choice.wrappedValue
        return HStack(alignment: .firstTextBaseline, spacing: 10) {
            Text(label)
            VStack(alignment: .leading, spacing: 4) {
                DATE_COMBO(text: text, handle: handle, changed: { value in
                    if DATE_COMBO.FORMATS.contains(value) {
                        choice.wrappedValue = value
                    } else {
                        choice.wrappedValue = ""
                        custom.wrappedValue = value
                    }
                }, submit: {
                    MOD_ACTIONS.apply(main)
                }, cancel: {
                    MOD_ACTIONS.cancel(main)
                })
                if !problem.isEmpty {
                    Text(problem)
                        .font(.system(size: 11.5))
                        .foregroundStyle(Color(nsColor: PALETTE.ERROR_TEXT))
                        .fixedSize(horizontal: false, vertical: true)
                }
            }
        }
        .modifier(TOKEN_SETTINGS_BOX())
        .id(field)
    }
}
