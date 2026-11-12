import SwiftUI

import limenamer_view_model

struct MAIN_COMMANDS: Commands
{
    let main: MAIN_VIEW_MODEL

    var body: some Commands
    {
        CommandGroup(replacing: .toolbar) {
            Picker(STRINGS.theme_title, selection: Binding(get: {
                return main.settings.theme_mode
            }, set: { mode in
                main.set_theme(mode)
            })) {
                ForEach(MAIN_VIEW_MODEL.THEME_CHOICES, id: \.value) { choice in
                    Text(choice.label)
                        .tag(choice.value)
                }
            }
        }
    }
}
