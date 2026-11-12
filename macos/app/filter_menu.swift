import SwiftUI

import limenamer_view_model

struct FILTER_MENU: View
{
    private static let FILTERS: [VIEW_FILTER] = [.all, .changed, .warning, .error, .conflict]

    let list: ITEM_LIST_VIEW_MODEL

    var body: some View
    {
        ForEach(FILTER_MENU.FILTERS, id: \.self) { filter in
            Toggle(label(filter), isOn: Binding(get: {
                return list.filter == filter
            }, set: { _ in
                list.filter = filter
            }))
        }
    }

    private func label(_ filter: VIEW_FILTER) -> String
    {
        return JOSA_FORMATTER.format(STRINGS.filter_item, ITEM_LIST_VIEW_MODEL.filter_name(filter),
            count(filter))
    }

    private func count(_ filter: VIEW_FILTER) -> Int
    {
        switch filter {
        case .all:
            return list.count_all
        case .changed:
            return Int(list.count_changed)
        case .warning:
            return Int(list.count_warning)
        case .error:
            return Int(list.count_error)
        case .conflict:
            return Int(list.count_conflict)
        }
    }
}
