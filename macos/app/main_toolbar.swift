import SwiftUI

import limenamer_view_model

struct MAIN_TOOLBAR: ToolbarContent
{
    let main: MAIN_VIEW_MODEL

    var body: some ToolbarContent
    {
        ToolbarItemGroup(placement: .primaryAction) {
            Menu {
                Button(STRINGS.add_files, systemImage: "doc") {
                    add(folders: false, mode: .as_items)
                }
                Button(STRINGS.add_folders, systemImage: "folder") {
                    add(folders: true, mode: .as_items)
                }
                Button(STRINGS.add_folder_contents, systemImage: "doc.on.doc") {
                    add(folders: true, mode: .folder_contents)
                }
            } label: {
                Label(STRINGS.toolbar_add, systemImage: "plus")
            } primaryAction: {
                add(folders: false, mode: .as_items)
            }
            .help(STRINGS.toolbar_add)
            .disabled(!main.can_edit)
            Button(STRINGS.toolbar_remove, systemImage: "minus") {
                main.remove_selected()
            }
            .help(STRINGS.toolbar_remove)
            .disabled(!main.can_remove)
            Button(STRINGS.toolbar_clear, systemImage: "trash") {
                main.clear()
            }
            .help(STRINGS.toolbar_clear)
            .disabled(!main.can_clear)
        }
        if #available(macOS 26.0, *) {
            ToolbarSpacer(.fixed, placement: .primaryAction)
        }
        ToolbarItemGroup(placement: .primaryAction) {
            Button(STRINGS.toolbar_undo, systemImage: "arrow.uturn.backward") {
                main.undo()
            }
            .help(STRINGS.toolbar_undo)
            .disabled(!main.can_undo)
            Button(STRINGS.toolbar_redo, systemImage: "arrow.uturn.forward") {
                main.redo()
            }
            .help(STRINGS.toolbar_redo)
            .disabled(!main.can_redo)
        }
        if #available(macOS 26.0, *) {
            ToolbarSpacer(.fixed, placement: .primaryAction)
        }
        ToolbarItemGroup(placement: .primaryAction) {
            Menu {
                SORT_MENU(main: main)
            } label: {
                Label(STRINGS.toolbar_sort, systemImage: "arrow.up.arrow.down")
            }
            .help(STRINGS.toolbar_sort)
            .disabled(!main.can_edit)
            Menu {
                FILTER_MENU(list: main.list)
            } label: {
                Label(STRINGS.toolbar_filter, systemImage: "line.3.horizontal.decrease.circle")
            }
            .help(STRINGS.toolbar_filter)
            .disabled(!main.can_edit)
            Button(STRINGS.toolbar_duplicate, systemImage: "plus.square.on.square") {
            }
            .help(STRINGS.toolbar_duplicate)
            .disabled(true)
        }
        if #available(macOS 26.0, *) {
            ToolbarSpacer(.fixed, placement: .primaryAction)
        }
        ToolbarItem(placement: .primaryAction) {
            Toggle(isOn: panel_open) {
                Label(STRINGS.toolbar_panel, systemImage: "sidebar.right")
            }
            .toggleStyle(.button)
            .overlay(alignment: .topTrailing) {
                if main.shows_panel_badge {
                    COUNT_BADGE(count: main.mods.enabled_count)
                        .offset(x: 4, y: -4)
                        .allowsHitTesting(false)
                }
            }
            .help(STRINGS.toolbar_panel)
            .disabled(!main.can_toggle_panel)
        }
    }

    private var panel_open: Binding<Bool>
    {
        return Binding(get: {
            return main.panel_open
        }, set: { _ in
            main.toggle_panel()
        })
    }

    private func add(folders: Bool, mode: ADD_MODE)
    {
        Task {
            let paths: [String] = await FILE_PICKER.choose(folders: folders)
            main.add_paths(paths, mode: mode)
        }
    }
}
