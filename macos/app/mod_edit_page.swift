import SwiftUI

import limenamer_view_model

struct MOD_EDIT_PAGE: View
{
    private static let BUTTON_WIDTH: CGFloat = 72

    let main: MAIN_VIEW_MODEL

    @FocusState private var focus: MOD_FIELD?
    @State private var token_handle: VIEW_HANDLE = VIEW_HANDLE()
    @State private var modified_handle: VIEW_HANDLE = VIEW_HANDLE()
    @State private var created_handle: VIEW_HANDLE = VIEW_HANDLE()

    var body: some View
    {
        VStack(spacing: 0) {
            PANEL_HEADER(leading: 8) {
                BACK_BUTTON(label: main.mods.back_label) {
                    main.mods.go_back()
                }
                Spacer()
                STATE_BADGE(text: main.mods.badge_text, existing: main.mods.is_existing)
            }
            ScrollViewReader { proxy in
                ScrollView {
                    MOD_EDIT_FORM(main: main, editor: main.mods.editor,
                        handles: (token_handle, modified_handle, created_handle),
                        focus: $focus)
                        .padding(EdgeInsets(top: 4, leading: 16, bottom: 16, trailing: 16))
                }
                .onAppear {
                    connect(proxy)
                }
            }
            Divider()
            MOD_EXAMPLE_VIEW(main: main)
            Divider()
            buttons
        }
    }

    private var buttons: some View
    {
        HStack(spacing: 8) {
            Spacer()
            Button {
                MOD_ACTIONS.cancel(main)
            } label: {
                Text(STRINGS.button_cancel)
                    .frame(minWidth: MOD_EDIT_PAGE.BUTTON_WIDTH)
            }
            .keyboardShortcut(.cancelAction)
            Button {
                MOD_ACTIONS.apply(main)
            } label: {
                Text(main.mods.apply_label)
                    .frame(minWidth: MOD_EDIT_PAGE.BUTTON_WIDTH)
            }
            .keyboardShortcut(.defaultAction)
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 12)
    }

    private func connect(_ proxy: ScrollViewProxy)
    {
        let binding: FocusState<MOD_FIELD?>.Binding = $focus
        let token: VIEW_HANDLE = token_handle
        let modified: VIEW_HANDLE = modified_handle
        let created: VIEW_HANDLE = created_handle
        main.mods.editor.focus_requested = { field in
            proxy.scrollTo(field, anchor: .center)
            switch field {
            case .value:
                token.focus()
            case .modified_date_format:
                modified.focus()
            case .created_date_format:
                created.focus()
            default:
                binding.wrappedValue = field
            }
        }
    }
}
