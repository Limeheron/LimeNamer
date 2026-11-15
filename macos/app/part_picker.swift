import SwiftUI

import limenamer_view_model

struct PART_PICKER: View
{
    private static let CHOICES: [CHOICE<PART>] = MOD_EDITOR_VIEW_MODEL.PART_CHOICES

    let part: PART
    let changed: (PART) -> Void

    var body: some View
    {
        let choices: [CHOICE<PART>] = PART_PICKER.CHOICES
        SEGMENTS(labels: choices.map { $0.label },
            tips: choices.map { MOD_EDITOR_VIEW_MODEL.part_tip($0.value) },
            selected: choices.firstIndex { $0.value == part } ?? 0,
            changed: { index in
                if index >= 0 && index < choices.count {
                    changed(choices[index].value)
                }
            })
    }
}
