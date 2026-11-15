import SwiftUI

import limenamer_view_model

struct LIST_GUIDE: View
{
    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        if main.list.shows_empty_list {
            ContentUnavailableView {
                Label(STRINGS.empty_list_title, systemImage: "folder")
            } description: {
                Text(STRINGS.empty_list_text)
            } actions: {
                Button(STRINGS.toolbar_add) {
                    FILE_PICKER.add(main, folders: false, mode: .as_items)
                }
                .disabled(!main.can_edit)
            }
        } else if main.list.shows_empty_filter {
            ContentUnavailableView {
                Label(main.list.empty_filter_title,
                    systemImage: "line.3.horizontal.decrease.circle")
            } actions: {
                Button(STRINGS.show_all) {
                    main.list.filter = .all
                }
            }
        }
    }
}
