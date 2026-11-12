import SwiftUI

import limenamer_view_model

@main
struct LIMENAMER_APP: App
{
    static let VERSION_TOKEN: String = "LIMENAMER_VERSION=0.1.0"
    static let WINDOW_ID: String = "main"

    @State private var main: MAIN_VIEW_MODEL = LIMENAMER_APP.make_main()

    init()
    {
        UserDefaults.standard.removeObject(forKey: "NSWindow Frame \(LIMENAMER_APP.WINDOW_ID)")
    }

    var body: some Scene
    {
        Window("LimeNamer", id: LIMENAMER_APP.WINDOW_ID) {
            MAIN_WINDOW(main: main)
        }
        .defaultSize(width: 1280, height: 800)
        .windowResizability(.contentMinSize)
        .commands {
            MAIN_COMMANDS(main: main)
        }
    }

    private static func make_main() -> MAIN_VIEW_MODEL
    {
        let engine: ENGINE
        do throws(ENGINE_ERROR) {
            engine = try ENGINE()
        } catch {
            fatalError("Cannot start the engine: \(error.result)")
        }
        let settings: SETTINGS = SETTINGS(store: UserDefaults.standard)
        return MAIN_VIEW_MODEL(engine: engine, settings: settings, dialogs: ALERT_DIALOGS(),
            now: { Date() })
    }
}
