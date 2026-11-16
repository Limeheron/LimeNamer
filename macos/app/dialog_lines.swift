import AppKit
import SwiftUI

import limenamer_view_model

struct DIALOG_LINES: View
{
    private static let MAX_HEIGHT: CGFloat = 220
    private static let RADIUS: CGFloat = 8

    let lines: [DIALOG_LINE]

    @State private var height: CGFloat = 0

    var body: some View
    {
        let shape: RoundedRectangle = RoundedRectangle(cornerRadius: DIALOG_LINES.RADIUS)
        ScrollView {
            VStack(alignment: .leading, spacing: 0) {
                ForEach(Array(lines.enumerated()), id: \.offset) { offset, line in
                    row(line)
                    if offset < lines.count - 1 {
                        Divider()
                    }
                }
            }
            .background(GeometryReader { proxy in
                Color.clear
                    .onAppear {
                        height = proxy.size.height
                    }
                    .onChange(of: proxy.size.height) { _, current in
                        height = current
                    }
            })
        }
        .frame(height: min(height, DIALOG_LINES.MAX_HEIGHT))
        .background(shape.fill(Color(nsColor: .controlBackgroundColor)))
        .overlay(shape.strokeBorder(Color(nsColor: .separatorColor), lineWidth: 1))
        .clipShape(shape)
    }

    private func row(_ line: DIALOG_LINE) -> some View
    {
        let warning: Bool = line.icon == .warning
        return HStack(alignment: .firstTextBaseline, spacing: 8) {
            Image(systemName: warning ? "exclamationmark.triangle" : "xmark.circle")
                .font(.system(size: 11, weight: .semibold))
                .foregroundStyle(Color(nsColor: warning ? PALETTE.WARNING_TEXT
                    : PALETTE.ERROR_TEXT))
                .frame(width: 14)
            Text(line.index > 0 ? String(line.index) : "")
                .font(.system(size: 12).monospacedDigit())
                .foregroundStyle(.secondary)
                .frame(width: 24, alignment: .trailing)
            VStack(alignment: .leading, spacing: 2) {
                Text(line.text)
                    .font(.system(size: 12, design: .monospaced))
                    .lineLimit(1)
                    .truncationMode(.middle)
                    .help(line.text)
                if !line.reason.isEmpty {
                    Text(line.reason)
                        .font(.system(size: 11.5))
                        .foregroundStyle(.secondary)
                        .fixedSize(horizontal: false, vertical: true)
                }
            }
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        .padding(.vertical, 7)
        .padding(.horizontal, 10)
    }
}
