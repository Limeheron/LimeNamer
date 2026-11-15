import SwiftUI

import limenamer_view_model

struct MOD_ADD_PAGE: View
{
    static var HEADER_GAP: CGFloat
    {
        return 22
    }

    let main: MAIN_VIEW_MODEL

    var body: some View
    {
        let mods: MOD_LIST_VIEW_MODEL = main.mods
        let header: KIND_CARD = mods.kind_header
        VStack(spacing: 0) {
            PANEL_HEADER(leading: 8) {
                BACK_BUTTON(label: mods.add_back_label) {
                    mods.go_back()
                }
                Spacer()
                STATE_BADGE(text: mods.badge_text, existing: mods.is_existing)
            }
            ScrollView {
                VStack(alignment: .leading, spacing: 0) {
                    KIND_PICKER(kind: mods.add_kind) { kind in
                        mods.add_kind = kind
                    }
                    VStack(alignment: .leading, spacing: 8) {
                        Text(header.description)
                            .font(.system(size: 16, weight: .semibold))
                            .fixedSize(horizontal: false, vertical: true)
                        HStack(spacing: 6) {
                            Text(header.example_before)
                            Image(systemName: "arrow.right")
                                .font(.system(size: 9, weight: .semibold))
                                .foregroundStyle(.secondary)
                            Text(header.example_after)
                                .fontWeight(.bold)
                                .foregroundStyle(Color(nsColor: PALETTE.ACCENT_TEXT))
                        }
                        .font(.system(size: 12.5, design: .monospaced))
                        .lineLimit(1)
                    }
                    .padding(.horizontal, 2)
                    .padding(.top, MOD_ADD_PAGE.HEADER_GAP)
                    Divider()
                        .padding(.top, 20)
                    VStack(spacing: 8) {
                        ForEach(mods.position_cards, id: \.type) { choice in
                            POSITION_CARD_VIEW(choice: choice) {
                                mods.choose_position(choice.type)
                            }
                        }
                    }
                    .padding(.top, 16)
                }
                .padding(EdgeInsets(top: 4, leading: 16, bottom: 16, trailing: 16))
            }
        }
    }
}
