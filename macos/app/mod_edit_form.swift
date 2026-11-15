import SwiftUI

import limenamer_view_model

struct MOD_EDIT_FORM: View
{
    let main: MAIN_VIEW_MODEL
    @Bindable var editor: MOD_EDITOR_VIEW_MODEL
    let handles: (token: VIEW_HANDLE, modified: VIEW_HANDLE, created: VIEW_HANDLE)
    let focus: FocusState<MOD_FIELD?>.Binding

    var body: some View
    {
        VStack(alignment: .leading, spacing: 16) {
            Text(editor.summary)
                .font(.system(size: 13, weight: .semibold))
                .fixedSize(horizontal: false, vertical: true)
                .frame(maxWidth: .infinity, alignment: .leading)
                .padding(.vertical, 9)
                .padding(.horizontal, 12)
                .background(CARD_BOX())
            FORM_FIELD(label: STRINGS.field_scope) {
                Picker("", selection: $editor.scope) {
                    ForEach(MOD_EDITOR_VIEW_MODEL.SCOPE_CHOICES, id: \.label) { choice in
                        Text(choice.label)
                            .tag(choice.value)
                    }
                }
                .pickerStyle(.radioGroup)
                .horizontalRadioGroupLayout()
                .labelsHidden()
                if editor.scope == .items {
                    TEXT_INPUT(text: number_text, problem: editor.problem(.targets),
                        field: .targets, focus: focus)
                        .help(STRINGS.index_tip)
                }
            }
            FORM_FIELD(label: STRINGS.field_part) {
                PART_PICKER(part: editor.part) { part in
                    editor.part = part
                }
                PART_DEMO(part: editor.part)
            }
            MOD_POSITION_FIELDS(editor: editor, focus: focus)
            if editor.shows_value {
                MOD_VALUE_FIELDS(main: main, editor: editor, handles: handles)
            }
            if editor.shows_format {
                FORM_FIELD(label: STRINGS.field_format) {
                    CHOICE_MENU(choices: MOD_EDITOR_VIEW_MODEL.FORMAT_CHOICES,
                        selection: $editor.format_type)
                    if editor.shows_digits {
                        HStack(spacing: 10) {
                            Text(STRINGS.field_digits)
                            NUMBER_INPUT(value: Int(editor.format_digits),
                                range: MOD_POSITION_FIELDS.DIGITS) { value in
                                editor.format_digits = UInt32(value)
                            }
                        }
                    }
                }
            }
        }
    }

    private var number_text: Binding<String>
    {
        let list: ITEM_LIST_VIEW_MODEL = main.list
        return Binding(get: {
            return list.number_text
        }, set: { text in
            list.number_text = text
        })
    }
}
