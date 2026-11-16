import SwiftUI

import limenamer_view_model

struct DUPLICATE_POPOVER: View
{
    private static let WIDTH: CGFloat = 280

    @Bindable var main: MAIN_VIEW_MODEL

    var body: some View
    {
        VStack(alignment: .leading, spacing: 10) {
            HStack(spacing: 10) {
                Text(STRINGS.toolbar_duplicate)
                    .font(.system(size: 13, weight: .semibold))
                Spacer()
                Toggle(STRINGS.duplicate_on, isOn: $main.duplicate_enabled)
                    .toggleStyle(.switch)
                    .labelsHidden()
            }
            Text(STRINGS.duplicate_on_tip)
                .font(.system(size: 11.5))
                .foregroundStyle(.secondary)
                .fixedSize(horizontal: false, vertical: true)
            FORM_FIELD(label: STRINGS.duplicate_format_title) {
                CHOICE_MENU(choices: MAIN_VIEW_MODEL.DUPLICATE_FORMAT_CHOICES,
                    selection: $main.duplicate_format)
                if main.duplicate_is_custom {
                    TextField(STRINGS.duplicate_custom_hint, text: $main.duplicate_custom_text)
                        .textFieldStyle(.roundedBorder)
                        .autocorrectionDisabled()
                }
            }
            FORM_FIELD(label: STRINGS.duplicate_mode_title) {
                CHOICE_MENU(choices: MAIN_VIEW_MODEL.DUPLICATE_MODE_CHOICES,
                    selection: $main.duplicate_mode)
            }
        }
        .padding(14)
        .frame(width: DUPLICATE_POPOVER.WIDTH)
        .disabled(!main.can_edit)
    }
}
