import AppKit

import limenamer_view_model

enum STATUS_LOOK
{
    static func symbol(_ status: STATUS) -> String
    {
        switch status {
        case .ok:
            return "checkmark.circle"
        case .unchanged:
            return "minus.circle"
        case .warning:
            return "exclamationmark.triangle"
        case .error:
            return "xmark.circle"
        case .conflict:
            return "arrow.left.arrow.right"
        }
    }

    static func text(_ status: STATUS) -> NSColor
    {
        switch status {
        case .ok:
            return PALETTE.OK_TEXT
        case .unchanged:
            return .secondaryLabelColor
        case .warning:
            return PALETTE.WARNING_TEXT
        case .error:
            return PALETTE.ERROR_TEXT
        case .conflict:
            return PALETTE.CONFLICT_TEXT
        }
    }

    static func back(_ status: STATUS) -> NSColor
    {
        switch status {
        case .ok:
            return PALETTE.OK_BACK
        case .unchanged:
            return PALETTE.NEUTRAL_BACK
        case .warning:
            return PALETTE.WARNING_BACK
        case .error:
            return PALETTE.ERROR_BACK
        case .conflict:
            return PALETTE.CONFLICT_BACK
        }
    }
}
