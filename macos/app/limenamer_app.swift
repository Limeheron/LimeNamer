import SwiftUI

import limenamer_view_model

@main
struct LIMENAMER_APP: App
{
    static let VERSION_TOKEN: String = "LIMENAMER_VERSION=0.1.0"
    static let WINDOW_ID: String = "main"

    @State private var dialogs: SHEET_DIALOGS
    @State private var main: MAIN_VIEW_MODEL
    @State private var state: WINDOW_STATE = WINDOW_STATE()

    init()
    {
        UserDefaults.standard.removeObject(forKey: "NSWindow Frame \(LIMENAMER_APP.WINDOW_ID)")
        let dialogs: SHEET_DIALOGS = SHEET_DIALOGS()
        _dialogs = State(initialValue: dialogs)
        _main = State(initialValue: LIMENAMER_APP.make_main(dialogs))
    }

    var body: some Scene
    {
        Window("LimeNamer", id: LIMENAMER_APP.WINDOW_ID) {
            MAIN_WINDOW(main: main, dialogs: dialogs, state: state)
        }
        .defaultSize(width: 1280, height: 800)
        .windowResizability(.contentMinSize)
        .commands {
            MAIN_COMMANDS(main: main, state: state, version: LIMENAMER_APP.version)
        }
    }

    private static var version: String
    {
        return String(VERSION_TOKEN.split(separator: "=").last ?? "")
    }

    private static func make_main(_ dialogs: SHEET_DIALOGS) -> MAIN_VIEW_MODEL
    {
        let engine: ENGINE
        do throws(ENGINE_ERROR) {
            engine = try ENGINE()
        } catch {
            fatalError("Cannot start the engine: \(error.result)")
        }
        let settings: SETTINGS = SETTINGS(store: UserDefaults.standard)
        return MAIN_VIEW_MODEL(engine: engine, settings: settings, dialogs: dialogs,
            now: { Date() })
    }
}
