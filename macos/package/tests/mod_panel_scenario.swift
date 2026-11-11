import Testing

import limenamer_view_model

@MainActor
final class MOD_PANEL_SCENARIO
{
    let items: LIST_SCENARIO
    let mods: MOD_LIST_VIEW_MODEL
    private(set) var messages: [String] = []
    private(set) var focused: MOD_FIELD? = nil

    init(_ names: String...) throws
    {
        items = try LIST_SCENARIO(names: names)
        mods = MOD_LIST_VIEW_MODEL(engine: items.engine, list: items.list)
        mods.changed = { [unowned self] text in
            self.messages.append(text)
            do {
                try self.load()
            } catch {
                Issue.record(error)
            }
        }
        mods.notice = { [unowned self] text in
            self.messages.append(text)
        }
        mods.editor.focus_requested = { [unowned self] field in
            self.focused = field
        }
        try load()
    }

    var editor: MOD_EDITOR_VIEW_MODEL
    {
        return mods.editor
    }

    var engine: ENGINE
    {
        return items.engine
    }

    func load() throws
    {
        try items.load()
        try mods.load(try engine.preview())
    }

    func first_new_name() throws -> String
    {
        return try engine.preview().rows[0].new_name
    }

    func close()
    {
        items.close()
    }
}
