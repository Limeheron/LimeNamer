import SwiftUI

import limenamer_view_model

struct KIND_PICKER: View
{
    private static let CHOICES: [CHOICE<MOD_KIND>] = MOD_LIST_VIEW_MODEL.KIND_CHOICES

    let kind: MOD_KIND
    let changed: (MOD_KIND) -> Void

    var body: some View
    {
        let choices: [CHOICE<MOD_KIND>] = KIND_PICKER.CHOICES
        SEGMENTS(labels: choices.map { $0.label }, tips: [],
            selected: choices.firstIndex { $0.value == kind } ?? 0,
            changed: { index in
                if index >= 0 && index < choices.count {
                    changed(choices[index].value)
                }
            })
    }
}
