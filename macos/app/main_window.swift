import AppKit
import SwiftUI
import UniformTypeIdentifiers

import limenamer_view_model

struct MAIN_WINDOW: View
{
    private static let LIST_PADDING: CGFloat = 10
    private static let PANEL_WIDTH: CGFloat = 360
    private static let PANEL_MIN_WIDTH: CGFloat = 300
    private static let PROGRESS_INTERVAL: Duration = .milliseconds(50)

    let main: MAIN_VIEW_MODEL
    let dialogs: SHEET_DIALOGS
    let state: WINDOW_STATE

    @State private var panel_width: CGFloat = MAIN_WINDOW.PANEL_WIDTH
    @State private var dropping: Bool = false

    var body: some View
    {
        VStack(spacing: 0) {
            GeometryReader { geometry in
                let limit: ClosedRange<CGFloat> = MAIN_WINDOW.panel_limit(geometry.size.width)
                let width: CGFloat = min(max(panel_width, limit.lowerBound), limit.upperBound)
                HStack(spacing: 0) {
                    list_area
                    if main.panel_open {
                        PANEL_DIVIDER(width: $panel_width, current: width, limit: limit)
                            .zIndex(1)
                        panel_area
                            .frame(width: width)
                    }
                }
            }
            Divider()
            BOTTOM_BAR(main: main)
        }
        .frame(minWidth: 900, minHeight: 560)
        .background(WINDOW_SETUP(state: state))
        .navigationSubtitle(main.list.subtitle_text)
        .toolbar {
            MAIN_TOOLBAR(main: main, state: state)
        }
        .sheet(isPresented: dialog_shown) {
            if let content = dialogs.current {
                DIALOG_SHEET(content: content) { primary in
                    dialogs.answer(primary)
                }
            }
        }
        .onChange(of: main.settings.theme_mode, initial: true) { _, mode in
            NSApp.appearance = MAIN_WINDOW.appearance(mode)
        }
        .onReceive(NotificationCenter.default.publisher(
            for: NSApplication.didBecomeActiveNotification)) { _ in
            main.activated()
        }
        .onChange(of: main.is_running) { _, running in
            state.set_closable(!running)
        }
        .task(id: main.is_running) {
            while main.is_running && !Task.isCancelled {
                main.status.poll_progress()
                try? await Task.sleep(for: MAIN_WINDOW.PROGRESS_INTERVAL)
            }
        }
    }

    private var dialog_shown: Binding<Bool>
    {
        let dialogs: SHEET_DIALOGS = dialogs
        return Binding(get: {
            return dialogs.current != nil
        }, set: { _ in
        })
    }

    private var list_area: some View
    {
        ITEM_TABLE(main: main, rows: main.list.rows,
            hidden: ITEM_COLUMN.hidden(main.settings, designating: main.list.designating),
            checked: main.list.checked_ids)
            .padding(.horizontal, MAIN_WINDOW.LIST_PADDING)
            .overlay {
                if dropping {
                    DROP_GUIDE()
                } else {
                    LIST_GUIDE(main: main)
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(Color(nsColor: .controlBackgroundColor), ignoresSafeAreaEdges: [])
            .onDrop(of: [.fileURL], delegate: FILE_DROP(main: main, targeted: $dropping))
    }

    private var panel_area: some View
    {
        MOD_PANEL(main: main)
    }

    private static func panel_limit(_ total_width: CGFloat) -> ClosedRange<CGFloat>
    {
        return PANEL_MIN_WIDTH...max(PANEL_MIN_WIDTH, total_width / 2)
    }

    private static func appearance(_ mode: THEME_MODE) -> NSAppearance?
    {
        switch mode {
        case .system:
            return nil
        case .light:
            return NSAppearance(named: .aqua)
        case .dark:
            return NSAppearance(named: .darkAqua)
        }
    }
}
