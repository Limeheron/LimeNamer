import AppKit
import SwiftUI

import limenamer_view_model

struct MOD_PANEL: View
{
    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        Group {
            switch main.mods.page {
            case .list:
                MOD_LIST_PAGE(main: main)
            case .add:
                MOD_ADD_PAGE(main: main)
            case .edit:
                MOD_EDIT_PAGE(main: main)
            }
        }
        .disabled(!main.can_edit)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .top)
        .background(Color(nsColor: .windowBackgroundColor), ignoresSafeAreaEdges: [])
    }
}
