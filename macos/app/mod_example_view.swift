import SwiftUI

import limenamer_view_model

struct MOD_EXAMPLE_VIEW: View
{
    private static let LABEL_WIDTH: CGFloat = 22

    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        let editor: MOD_EDITOR_VIEW_MODEL = main.mods.editor
        let collapsed: Bool = main.settings.example_collapsed
        VStack(alignment: .leading, spacing: 6) {
            HStack(spacing: 8) {
                Text(STRINGS.example_title)
                    .font(.system(size: 12, weight: .semibold))
                    .foregroundStyle(.secondary)
                if editor.has_example {
                    Text(editor.example_index_text)
                        .font(.system(size: 12))
                        .foregroundStyle(.secondary)
                }
                Spacer()
                Button {
                    main.settings.example_collapsed.toggle()
                } label: {
                    Image(systemName: collapsed ? "chevron.down" : "chevron.up")
                }
                .buttonStyle(.accessoryBar)
                .help(STRINGS.example_toggle_tip)
            }
            if !collapsed && editor.has_example {
                Grid(alignment: .leading, horizontalSpacing: 6, verticalSpacing: 4) {
                    GridRow {
                        label(STRINGS.example_before)
                        NAME_LINE(parts: editor.example_before, mark: PALETTE.DELETE_BACK)
                    }
                    GridRow {
                        label(STRINGS.example_after)
                        NAME_LINE(parts: editor.example_after, mark: PALETTE.MARK_BACK)
                    }
                    if !editor.example_state_text.isEmpty {
                        GridRow {
                            Color.clear
                                .frame(width: 1, height: 1)
                            Text(editor.example_state_text)
                                .font(.system(size: 12))
                                .foregroundStyle(Color(nsColor: PALETTE.WARNING_TEXT))
                        }
                    }
                }
            }
        }
        .padding(EdgeInsets(top: 8, leading: 16, bottom: 10, trailing: 12))
    }

    private func label(_ text: String) -> some View
    {
        return Text(text)
            .font(.system(size: 12))
            .foregroundStyle(.secondary)
            .frame(width: MOD_EXAMPLE_VIEW.LABEL_WIDTH, alignment: .leading)
    }
}
