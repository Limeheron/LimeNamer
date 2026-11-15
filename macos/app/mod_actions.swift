import limenamer_view_model

@MainActor
enum MOD_ACTIONS
{
    static func run(_ main: MAIN_VIEW_MODEL, _ action: () throws(ENGINE_ERROR) -> Void)
    {
        do throws(ENGINE_ERROR) {
            try action()
        } catch {
            main.status.post(MESSAGES.result_text(error.result))
        }
    }

    static func apply(_ main: MAIN_VIEW_MODEL)
    {
        if main.mods.page == .edit {
            run(main) { () throws(ENGINE_ERROR) in
                _ = try main.mods.apply()
            }
        }
    }

    static func cancel(_ main: MAIN_VIEW_MODEL)
    {
        if main.mods.page == .edit {
            main.mods.cancel()
        }
    }
}
