import SwiftUI

import limenamer_view_model

struct MOD_ADD_PAGE: View
{
    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        VStack(spacing: 0) {
            PANEL_HEADER(leading: 8) {
                BACK_BUTTON(label: STRINGS.back_to_list) {
                    main.mods.close_add()
                }
                Spacer()
            }
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    FORM_FIELD(label: STRINGS.add_part_title) {
                        PART_PICKER(part: main.mods.add_part) { part in
                            main.mods.add_part = part
                        }
                    }
                    FORM_FIELD(label: STRINGS.add_kind_title) {
                        VStack(spacing: 8) {
                            ForEach(main.mods.kind_cards, id: \.kind) { card in
                                KIND_CARD_VIEW(card: card) {
                                    main.mods.choose_kind(card.kind)
                                }
                            }
                        }
                    }
                }
                .padding(EdgeInsets(top: 4, leading: 16, bottom: 16, trailing: 16))
            }
        }
    }
}
