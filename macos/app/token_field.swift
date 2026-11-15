import AppKit
import SwiftUI

import limenamer_view_model

struct TOKEN_FIELD: NSViewRepresentable
{
    let pieces: [VALUE_PIECE]
    let handle: VIEW_HANDLE
    let changed: ([VALUE_PIECE]) -> Void
    let focus_changed: (Bool) -> Void
    let submit: () -> Void
    let cancel: () -> Void

    func makeNSView(context: Context) -> TOKEN_TEXT_VIEW
    {
        let view: TOKEN_TEXT_VIEW = TOKEN_TEXT_VIEW()
        handle.view = view
        updateNSView(view, context: context)
        return view
    }

    func updateNSView(_ view: TOKEN_TEXT_VIEW, context: Context)
    {
        view.changed = changed
        view.focus_changed = focus_changed
        view.submit = submit
        view.cancel = cancel
        view.isEditable = context.environment.isEnabled
        view.set_pieces(pieces)
    }

    func sizeThatFits(_ proposal: ProposedViewSize, nsView view: TOKEN_TEXT_VIEW,
        context: Context) -> CGSize?
    {
        let width: CGFloat = proposal.width ?? view.bounds.width
        return CGSize(width: width, height: view.height(for: width))
    }
}
