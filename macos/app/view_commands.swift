import SwiftUI

import limenamer_view_model

struct VIEW_COMMANDS: Commands
{
    let main: MAIN_VIEW_MODEL
    let state: WINDOW_STATE

    var body: some Commands
    {
        CommandGroup(replacing: .toolbar) {
            Group {
                Menu(STRINGS.sort_title) {
                    SORT_MENU(main: main)
                }
                Menu(STRINGS.filter_title) {
                    FILTER_MENU(list: main.list)
                }
            }
            .disabled(!main.can_edit || state.sheet_open)
            Menu(STRINGS.toolbar_columns) {
                ForEach(ITEM_COLUMN.HIDEABLE, id: \.self) { column in
                    Toggle(column.title, isOn: Binding(get: {
                        return column.shown(main.settings)
                    }, set: { shown in
                        column.set_shown(main.settings, shown)
                    }))
                }
            }
            Divider()
            Button(main.panel_open ? STRINGS.menu_panel_hide : STRINGS.menu_panel_show) {
                main.toggle_panel()
            }
            .keyboardShortcut("i", modifiers: [.command, .option])
            .disabled(!main.can_toggle_panel || state.sheet_open)
            Button(STRINGS.menu_duplicate) {
                state.duplicate_shown = true
            }
            .disabled(!main.can_edit || state.sheet_open)
            Divider()
            Picker(STRINGS.theme_title, selection: Binding(get: {
                return main.settings.theme_mode
            }, set: { mode in
                main.set_theme(mode)
            })) {
                ForEach(MAIN_VIEW_MODEL.THEME_CHOICES, id: \.value) { choice in
                    Text(choice.label)
                        .tag(choice.value)
                }
            }
        }
    }
}
