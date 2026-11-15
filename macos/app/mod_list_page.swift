import SwiftUI

import limenamer_view_model

struct MOD_LIST_PAGE: View
{
    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        VStack(spacing: 0) {
            PANEL_HEADER(leading: 16) {
                if main.mods.has_mods {
                    MIXED_CHECK_BOX(state: main.mods.all_enabled_state,
                        tip: STRINGS.mods_toggle_all_tip) {
                        MOD_ACTIONS.run(main) { () throws(ENGINE_ERROR) in
                            try main.mods.toggle_all()
                        }
                    }
                    .fixedSize()
                }
                Text(STRINGS.panel_title)
                    .font(.system(size: 13, weight: .bold))
                Spacer()
                Button {
                    main.mods.open_add()
                } label: {
                    Image(systemName: "plus")
                }
                .buttonStyle(.bordered)
                .buttonBorderShape(.circle)
                .help(STRINGS.panel_add)
            }
            if main.mods.has_mods {
                MOD_CARD_TABLE(main: main, cards: main.mods.cards)
            } else {
                ContentUnavailableView {
                    Label(STRINGS.mods_empty_title, systemImage: "slider.horizontal.3")
                } description: {
                    Text(STRINGS.mods_empty_text)
                } actions: {
                    Button(STRINGS.panel_add) {
                        main.mods.open_add()
                    }
                }
                .frame(maxHeight: .infinity)
            }
        }
    }
}
