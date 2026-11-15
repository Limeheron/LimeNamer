import AppKit

import limenamer_view_model

enum ITEM_COLUMN: String, CaseIterable
{
    case check
    case index
    case status
    case current
    case new_name
    case folder
    case modified
    case created
    case size

    static let HIDEABLE: [ITEM_COLUMN] = [.index, .status, .folder, .modified, .created, .size]

    var identifier: NSUserInterfaceItemIdentifier
    {
        return NSUserInterfaceItemIdentifier(rawValue)
    }

    var title: String
    {
        switch self {
        case .check:
            return ""
        case .index:
            return STRINGS.column_index
        case .status:
            return STRINGS.column_status
        case .current:
            return STRINGS.column_current
        case .new_name:
            return STRINGS.column_new
        case .folder:
            return STRINGS.column_folder
        case .modified:
            return STRINGS.column_modified
        case .created:
            return STRINGS.column_created
        case .size:
            return STRINGS.column_size
        }
    }

    var width: CGFloat
    {
        switch self {
        case .check:
            return 36
        case .index:
            return 48
        case .status:
            return 104
        case .current, .new_name:
            return 240
        case .folder:
            return 200
        case .modified, .created:
            return 132
        case .size:
            return 80
        }
    }

    var min_width: CGFloat
    {
        switch self {
        case .check:
            return 36
        case .index:
            return 40
        case .status:
            return 80
        case .current, .new_name:
            return 120
        case .folder:
            return 80
        case .modified, .created:
            return 100
        case .size:
            return 60
        }
    }

    var flexible: Bool
    {
        return self == .current || self == .new_name
    }

    var alignment: NSTextAlignment
    {
        switch self {
        case .check, .index:
            return .center
        case .size:
            return .right
        default:
            return .left
        }
    }

    @MainActor
    static func hidden(_ settings: SETTINGS, designating: Bool) -> Set<ITEM_COLUMN>
    {
        var result: Set<ITEM_COLUMN> = designating ? [] : [.check]
        for column in HIDEABLE where !column.shown(settings, designating: designating) {
            result.insert(column)
        }
        return result
    }

    @MainActor
    func shown(_ settings: SETTINGS, designating: Bool = false) -> Bool
    {
        switch self {
        case .check:
            return designating
        case .index:
            return settings.show_index || designating
        case .status:
            return settings.show_status
        case .current, .new_name:
            return true
        case .folder:
            return settings.show_folder
        case .modified:
            return settings.show_modified
        case .created:
            return settings.show_created
        case .size:
            return settings.show_size
        }
    }

    @MainActor
    func set_shown(_ settings: SETTINGS, _ value: Bool)
    {
        switch self {
        case .index:
            settings.show_index = value
        case .status:
            settings.show_status = value
        case .check, .current, .new_name:
            break
        case .folder:
            settings.show_folder = value
        case .modified:
            settings.show_modified = value
        case .created:
            settings.show_created = value
        case .size:
            settings.show_size = value
        }
    }
}
