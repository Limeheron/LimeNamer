import SwiftUI

import limenamer_view_model

struct POSITION_CARD_VIEW: View
{
    let choice: POSITION_CHOICE
    let action: () -> Void

    @State private var hovering: Bool = false

    var body: some View
    {
        Button(action: action) {
            VStack(alignment: .leading, spacing: 3) {
                Text(choice.label)
                    .font(.system(size: 13, weight: .semibold))
                Text(choice.description)
                    .font(.system(size: 12))
                    .foregroundStyle(.secondary)
                HStack(spacing: 6) {
                    SAMPLE_TEXT(pieces: choice.before)
                    if choice.shows_after {
                        Image(systemName: "arrow.right")
                            .font(.system(size: 9, weight: .semibold))
                            .foregroundStyle(.secondary)
                        SAMPLE_TEXT(pieces: choice.after)
                    }
                }
                .padding(.top, 3)
            }
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(.vertical, 10)
            .padding(.horizontal, 12)
            .background(CARD_BOX(hovering: hovering))
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .onHover { inside in
            hovering = inside
        }
    }
}
