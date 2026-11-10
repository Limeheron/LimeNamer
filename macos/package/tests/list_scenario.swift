import limenamer_view_model

@MainActor
final class LIST_SCENARIO
{
    let temp: TEMP_FOLDER
    let engine: ENGINE
    let list: ITEM_LIST_VIEW_MODEL

    init(_ names: String...) throws
    {
        temp = try TEMP_FOLDER()
        engine = try ENGINE()
        list = ITEM_LIST_VIEW_MODEL(engine: engine)
        _ = try engine.items_add(try names.map { try temp.file($0) }, mode: .as_items)
        try load()
    }

    func id(_ name: String) throws -> UInt64
    {
        guard let row = try engine.preview().rows.first(where: { $0.name == name }) else {
            throw ENGINE_ERROR(result: .not_found)
        }
        return row.item_id
    }

    func load() throws
    {
        list.load(try engine.preview())
    }

    func visible_names() -> [String]
    {
        return list.rows.map { $0.name }
    }

    func close()
    {
        temp.remove()
    }
}
