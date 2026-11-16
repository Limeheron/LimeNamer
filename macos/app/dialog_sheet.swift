import AppKit
import SwiftUI

import limenamer_view_model

struct DIALOG_SHEET: View
{
    private static let WIDTH: CGFloat = 420
    private static let ICON: CGFloat = 48
    private static let BUTTON_WIDTH: CGFloat = 72

    let content: DIALOG_CONTENT
    let answer: (Bool) -> Void

    var body: some View
    {
        VStack(alignment: .leading, spacing: 16) {
            HStack(alignment: .top, spacing: 16) {
                Image(nsImage: NSApp.applicationIconImage)
                    .resizable()
                    .frame(width: DIALOG_SHEET.ICON, height: DIALOG_SHEET.ICON)
                VStack(alignment: .leading, spacing: 8) {
                    Text(content.title)
                        .font(.system(size: 13, weight: .bold))
                        .fixedSize(horizontal: false, vertical: true)
                    if !content.message.isEmpty {
                        Text(content.message)
                            .font(.system(size: 12))
                            .fixedSize(horizontal: false, vertical: true)
                    }
                    if !content.lines.isEmpty {
                        DIALOG_LINES(lines: content.lines)
                    }
                    if !content.note.isEmpty {
                        Text(content.note)
                            .font(.system(size: 11.5))
                            .foregroundStyle(.secondary)
                            .fixedSize(horizontal: false, vertical: true)
                    }
                }
            }
            HStack(spacing: 8) {
                Spacer()
                buttons
            }
        }
        .padding(EdgeInsets(top: 20, leading: 20, bottom: 18, trailing: 20))
        .frame(width: DIALOG_SHEET.WIDTH)
        .interactiveDismissDisabled()
        .onExitCommand {
            if content.kind != .choice {
                answer(content.kind == .report)
            }
        }
    }

    @ViewBuilder private var buttons: some View
    {
        switch content.kind {
        case .confirm:
            button(content.secondary, primary: false)
                .keyboardShortcut(.cancelAction)
            button(content.primary, primary: true)
                .keyboardShortcut(.defaultAction)
        case .choice:
            button(content.primary, primary: true)
            button(content.secondary, primary: false)
        case .report:
            button(content.primary, primary: true)
                .keyboardShortcut(.defaultAction)
        }
    }

    private func button(_ label: String, primary: Bool) -> some View
    {
        return Button {
            answer(primary)
        } label: {
            Text(label)
                .frame(minWidth: DIALOG_SHEET.BUTTON_WIDTH)
        }
    }
}
