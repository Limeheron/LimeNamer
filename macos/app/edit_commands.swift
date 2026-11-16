import AppKit
import SwiftUI

import limenamer_view_model

struct EDIT_COMMANDS: Commands
{
    let main: MAIN_VIEW_MODEL
    let state: WINDOW_STATE

    var body: some Commands
    {
        CommandGroup(replacing: .undoRedo) {
            Button(STRINGS.toolbar_undo) {
                if state.text_editing {
                    state.undo_text()
                } else {
                    main.undo()
                }
            }
            .keyboardShortcut("z")
            .disabled(state.text_editing ? !state.text_can_undo
                : !main.can_undo || state.sheet_open)
            Button(STRINGS.toolbar_redo) {
                if state.text_editing {
                    state.redo_text()
                } else {
                    main.redo()
                }
            }
            .keyboardShortcut("z", modifiers: [.command, .shift])
            .disabled(state.text_editing ? !state.text_can_redo
                : !main.can_redo || state.sheet_open)
        }
        CommandGroup(after: .pasteboard) {
            Divider()
            Button(STRINGS.row_direct_edit) {
                NSApp.sendAction(#selector(ITEM_TABLE_VIEW.edit_name(_:)), to: nil, from: nil)
            }
            .keyboardShortcut(.return, modifiers: [])
            .disabled(!state.list_focused || !main.can_edit || main.list.selected_ids.isEmpty)
            Button(STRINGS.row_remove) {
                main.remove_selected()
            }
            .keyboardShortcut(.delete, modifiers: [])
            .disabled(!state.list_focused || !main.can_remove)
            Button(STRINGS.menu_clear) {
                main.clear()
            }
            .disabled(!main.can_clear || state.sheet_open)
        }
    }
}
