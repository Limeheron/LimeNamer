import AppKit
import SwiftUI

import limenamer_view_model

struct BOTTOM_BAR: View
{
    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        let status: STATUS_BAR_VIEW_MODEL = main.status
        HStack(spacing: 10) {
            if status.progress_visible {
                Text(status.progress_label)
                    .font(.callout)
                Text(status.progress_text)
                    .font(.callout)
                    .monospacedDigit()
                    .foregroundStyle(.secondary)
                ProgressView(value: status.progress_value)
                    .progressViewStyle(.linear)
                    .tint(Color(nsColor: PALETTE.ACCENT_TEXT))
                    .frame(width: 160)
                Spacer(minLength: 0)
            } else {
                if !status.time_text.isEmpty {
                    Text(status.time_text)
                        .font(.callout)
                        .monospacedDigit()
                        .foregroundStyle(.secondary)
                }
                Text(status.message)
                    .font(.callout)
                    .lineLimit(1)
                    .truncationMode(.tail)
                    .help(status.message)
                Spacer(minLength: 0)
                pills
            }
            Button(STRINGS.button_rename_undo) {
                Task {
                    await main.rename_undo()
                }
            }
            .disabled(!main.can_rename_undo)
            Button(STRINGS.button_rename) {
                Task {
                    await main.rename()
                }
            }
            .buttonStyle(.borderedProminent)
            .disabled(!main.can_rename)
        }
        .controlSize(.large)
        .padding(.leading, 16)
        .padding(.trailing, 12)
        .frame(height: 48)
        .background(Color(nsColor: .windowBackgroundColor))
    }

    private var pills: some View
    {
        let status: STATUS_BAR_VIEW_MODEL = main.status
        return HStack(spacing: 2) {
            if status.has_errors {
                PILL_BUTTON(text: status.error_text, status: .error,
                    active: status.active_filter == .error) {
                    status.toggle_error()
                }
            }
            if status.has_conflicts {
                PILL_BUTTON(text: status.conflict_text, status: .conflict,
                    active: status.active_filter == .conflict) {
                    status.toggle_conflict()
                }
            }
            if status.has_warnings {
                PILL_BUTTON(text: status.warning_text, status: .warning,
                    active: status.active_filter == .warning) {
                    status.toggle_warning()
                }
            }
        }
    }
}
