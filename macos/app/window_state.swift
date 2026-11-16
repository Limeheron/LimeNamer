import AppKit
import Observation

@MainActor
@Observable
final class WINDOW_STATE: NSObject
{
    private static let NOTIFICATIONS: [Notification.Name] = [
        NSWindow.didBecomeKeyNotification,
        NSWindow.didResignKeyNotification,
        NSWindow.willBeginSheetNotification,
        NSWindow.didEndSheetNotification,
        .NSUndoManagerCheckpoint,
        .NSUndoManagerDidUndoChange,
        .NSUndoManagerDidRedoChange,
    ]

    var duplicate_shown: Bool = false
    private(set) var text_editing: Bool = false
    private(set) var text_can_undo: Bool = false
    private(set) var text_can_redo: Bool = false
    private(set) var list_focused: Bool = false
    private(set) var sheet_open: Bool = false

    @ObservationIgnored private weak var window: NSWindow? = nil
    @ObservationIgnored private weak var key_window: NSWindow? = nil
    @ObservationIgnored private var responder_observation: NSKeyValueObservation? = nil
    @ObservationIgnored private var refresh_pending: Bool = false

    override init()
    {
        super.init()
        for name in WINDOW_STATE.NOTIFICATIONS {
            NotificationCenter.default.addObserver(self, selector: #selector(changed(_:)),
                name: name, object: nil)
        }
    }

    func attach(_ window: NSWindow)
    {
        self.window = window
        schedule_refresh()
    }

    func undo_text()
    {
        WINDOW_STATE.editing_text()?.undoManager?.undo()
    }

    func redo_text()
    {
        WINDOW_STATE.editing_text()?.undoManager?.redo()
    }

    @objc private func changed(_ notification: Notification)
    {
        schedule_refresh()
    }

    private func schedule_refresh()
    {
        if refresh_pending {
            return
        }
        refresh_pending = true
        Task { @MainActor [weak self] in
            self?.refresh()
        }
    }

    private func refresh()
    {
        refresh_pending = false
        let key: NSWindow? = NSApp.keyWindow
        if key !== key_window {
            key_window = key
            responder_observation = key?.observe(\.firstResponder) { [weak self] _, _ in
                MainActor.assumeIsolated {
                    self?.schedule_refresh()
                }
            }
        }
        let text: NSTextView? = WINDOW_STATE.editing_text()
        let editing: Bool = text != nil
        let can_undo: Bool = text?.undoManager?.canUndo ?? false
        let can_redo: Bool = text?.undoManager?.canRedo ?? false
        let focused: Bool = key?.firstResponder is ITEM_TABLE_VIEW
        let sheet: Bool = window?.attachedSheet != nil
        if text_editing != editing {
            text_editing = editing
        }
        if text_can_undo != can_undo {
            text_can_undo = can_undo
        }
        if text_can_redo != can_redo {
            text_can_redo = can_redo
        }
        if list_focused != focused {
            list_focused = focused
        }
        if sheet_open != sheet {
            sheet_open = sheet
        }
    }

    private static func editing_text() -> NSTextView?
    {
        guard let view = NSApp.keyWindow?.firstResponder as? NSTextView, view.isEditable else {
            return nil
        }
        return view
    }
}
