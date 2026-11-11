import Observation

@MainActor
@Observable
public final class SETTINGS
{
    @ObservationIgnored private let store: any SETTINGS_STORE

    public init(store: any SETTINGS_STORE)
    {
        self.store = store
        show_index = SETTINGS.flag(store, "show_index", true)
        show_status = SETTINGS.flag(store, "show_status", true)
        show_folder = SETTINGS.flag(store, "show_folder", false)
        show_modified = SETTINGS.flag(store, "show_modified", true)
        show_created = SETTINGS.flag(store, "show_created", false)
        show_size = SETTINGS.flag(store, "show_size", false)
        let theme: String = store.object(forKey: "theme_mode") as? String ?? ""
        theme_mode = THEME_MODE(rawValue: theme) ?? .system
        example_collapsed = SETTINGS.flag(store, "example_collapsed", false)
        panel_open = SETTINGS.flag(store, "panel_open", true)
    }

    public var show_index: Bool = true
    {
        didSet {
            store.set(show_index, forKey: "show_index")
        }
    }

    public var show_status: Bool = true
    {
        didSet {
            store.set(show_status, forKey: "show_status")
        }
    }

    public var show_folder: Bool = false
    {
        didSet {
            store.set(show_folder, forKey: "show_folder")
        }
    }

    public var show_modified: Bool = true
    {
        didSet {
            store.set(show_modified, forKey: "show_modified")
        }
    }

    public var show_created: Bool = false
    {
        didSet {
            store.set(show_created, forKey: "show_created")
        }
    }

    public var show_size: Bool = false
    {
        didSet {
            store.set(show_size, forKey: "show_size")
        }
    }

    public var theme_mode: THEME_MODE = .system
    {
        didSet {
            store.set(theme_mode.rawValue, forKey: "theme_mode")
        }
    }

    public var example_collapsed: Bool = false
    {
        didSet {
            store.set(example_collapsed, forKey: "example_collapsed")
        }
    }

    public var panel_open: Bool = true
    {
        didSet {
            store.set(panel_open, forKey: "panel_open")
        }
    }

    private static func flag(_ store: any SETTINGS_STORE, _ key: String, _ fallback: Bool)
        -> Bool
    {
        return store.object(forKey: key) as? Bool ?? fallback
    }
}
