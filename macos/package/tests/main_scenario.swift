import Foundation

import limenamer_view_model

@MainActor
final class MAIN_SCENARIO
{
    let temp: TEMP_FOLDER
    let engine: ENGINE
    let store: MEMORY_STORE = MEMORY_STORE()
    let settings: SETTINGS
    let dialogs: FAKE_DIALOGS = FAKE_DIALOGS()
    let main: MAIN_VIEW_MODEL

    init(_ names: String...) throws
    {
        temp = try TEMP_FOLDER()
        engine = try ENGINE()
        settings = SETTINGS(store: store)
        let now = Date(timeIntervalSince1970: 1_794_206_520)
        main = MAIN_VIEW_MODEL(engine: engine, settings: settings, dialogs: dialogs,
            now: { now })
        if !names.isEmpty {
            main.add_paths(try names.map { try temp.file($0) }, mode: .as_items)
        }
    }

    var message: String
    {
        return main.status.message
    }

    func id(_ name: String) throws -> UInt64
    {
        guard let row = main.list.rows.first(where: { $0.name == name }) else {
            throw ENGINE_ERROR(result: .not_found)
        }
        return row.item_id
    }

    func add_mod(_ json: String) throws
    {
        _ = try engine.mod_add(json)
        main.refresh()
    }

    func close()
    {
        temp.remove()
    }
}
