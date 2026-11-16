import SwiftUI

import limenamer_view_model

struct FILE_COMMANDS: Commands
{
    let main: MAIN_VIEW_MODEL
    let state: WINDOW_STATE

    var body: some Commands
    {
        CommandGroup(replacing: .newItem) {
            Group {
                Button(STRINGS.add_files) {
                    FILE_PICKER.add(main, folders: false, mode: .as_items)
                }
                .keyboardShortcut("o")
                Button(STRINGS.add_folders) {
                    FILE_PICKER.add(main, folders: true, mode: .as_items)
                }
                Button(STRINGS.add_folder_contents) {
                    FILE_PICKER.add(main, folders: true, mode: .folder_contents)
                }
            }
            .disabled(!main.can_edit || state.sheet_open)
        }
        CommandGroup(after: .newItem) {
            Divider()
            Button(STRINGS.button_rename) {
                Task {
                    await main.rename()
                }
            }
            .disabled(!main.can_rename || state.sheet_open)
            Button(STRINGS.button_rename_undo) {
                Task {
                    await main.rename_undo()
                }
            }
            .disabled(!main.can_rename_undo || state.sheet_open)
        }
    }
}
