import SwiftUI

import limenamer_view_model

struct MOD_POSITION_FIELDS: View
{
    static let COUNT: ClosedRange<Int> = 1...999_999
    static let DIGITS: ClosedRange<Int> = 1...32
    private static let CHECK_WIDTH: CGFloat = 84

    @Bindable var editor: MOD_EDITOR_VIEW_MODEL
    let focus: FocusState<MOD_FIELD?>.Binding

    var body: some View
    {
        FORM_FIELD(label: STRINGS.field_position) {
            CHOICE_MENU(choices: editor.position_choices, selection: $editor.position_type)
            if editor.shows_n {
                HStack(spacing: 8) {
                    Text(STRINGS.field_n)
                    NUMBER_INPUT(value: Int(editor.n), range: MOD_POSITION_FIELDS.COUNT) { value in
                        editor.n = UInt32(value)
                    }
                    if editor.shows_count {
                        Text(STRINGS.field_m)
                            .padding(.leading, 8)
                        NUMBER_INPUT(value: Int(editor.count),
                            range: MOD_POSITION_FIELDS.COUNT) { value in
                            editor.count = UInt32(value)
                        }
                    }
                }
            }
        }
        if editor.shows_text {
            FORM_FIELD(label: STRINGS.field_find_text) {
                TEXT_INPUT(text: $editor.find_text, problem: editor.problem(.find_text),
                    field: .find_text, focus: focus)
            }
        }
        if editor.shows_between {
            FORM_FIELD(label: STRINGS.field_start_text) {
                HStack(alignment: .top, spacing: 10) {
                    TEXT_INPUT(text: $editor.start_text, problem: editor.problem(.start_text),
                        field: .start_text, focus: focus)
                        .disabled(!editor.shows_start_text)
                    MOD_POSITION_FIELDS.check(STRINGS.field_from_beginning,
                        $editor.from_beginning)
                }
            }
            FORM_FIELD(label: STRINGS.field_end_text) {
                HStack(alignment: .top, spacing: 10) {
                    TEXT_INPUT(text: $editor.end_text, problem: editor.problem(.end_text),
                        field: .end_text, focus: focus)
                        .disabled(!editor.shows_end_text)
                    MOD_POSITION_FIELDS.check(STRINGS.field_to_end, $editor.to_end)
                }
            }
            Toggle(STRINGS.field_include_delimiters, isOn: $editor.include_delimiters)
                .toggleStyle(.checkbox)
        }
        if editor.shows_occurrence {
            Toggle(STRINGS.field_case_sensitive, isOn: $editor.case_sensitive)
                .toggleStyle(.checkbox)
            FORM_FIELD(label: STRINGS.field_occurrence) {
                HStack(spacing: 8) {
                    CHOICE_MENU(choices: editor.occurrence_choices,
                        selection: $editor.occurrence)
                    if editor.shows_occurrence_n {
                        NUMBER_INPUT(value: Int(editor.occurrence_n),
                            range: MOD_POSITION_FIELDS.COUNT) { value in
                            editor.occurrence_n = UInt32(value)
                        }
                    }
                }
            }
        }
    }

    private static func check(_ label: String, _ value: Binding<Bool>) -> some View
    {
        return Toggle(label, isOn: value)
            .toggleStyle(.checkbox)
            .fixedSize()
            .frame(minWidth: CHECK_WIDTH, minHeight: FIELD_FRAME<EmptyView>.HEIGHT,
                alignment: .leading)
    }
}
