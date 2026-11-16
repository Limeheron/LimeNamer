import SwiftUI

import limenamer_view_model

struct MAIN_COMMANDS: Commands
{
    let main: MAIN_VIEW_MODEL
    let state: WINDOW_STATE
    let version: String

    var body: some Commands
    {
        CommandGroup(replacing: .appInfo) {
            Button(STRINGS.about_menu) {
                ABOUT_PANEL.show(version: version)
            }
        }
        FILE_COMMANDS(main: main, state: state)
        EDIT_COMMANDS(main: main, state: state)
        VIEW_COMMANDS(main: main, state: state)
        CommandGroup(replacing: .help) {
        }
    }
}
