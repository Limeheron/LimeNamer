import AppKit
import SwiftUI

import limenamer_view_model

struct KIND_CARD_VIEW: View
{
    let card: KIND_CARD
    let action: () -> Void

    @State private var hovering: Bool = false

    var body: some View
    {
        Button(action: action) {
            VStack(alignment: .leading, spacing: 3) {
                Text(card.name)
                    .font(.system(size: 13, weight: .semibold))
                Text(card.description)
                    .font(.system(size: 12))
                    .foregroundStyle(.secondary)
                HStack(spacing: 6) {
                    Text(card.example_before)
                    Image(systemName: "arrow.right")
                        .font(.system(size: 9, weight: .semibold))
                        .foregroundStyle(.secondary)
                    Text(card.example_after)
                        .foregroundStyle(Color(nsColor: PALETTE.ACCENT_TEXT))
                }
                .font(.system(size: 12, design: .monospaced))
                .lineLimit(1)
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
