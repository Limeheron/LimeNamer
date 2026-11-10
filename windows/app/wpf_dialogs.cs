using System.Windows;

namespace limenamer;

public sealed class WPF_DIALOGS : DIALOG_SERVICE
{
    private readonly Window owner_;

    public WPF_DIALOGS(Window owner)
    {
        owner_ = owner;
    }

    public bool ask(DIALOG_CONTENT content)
    {
        DIALOG_WINDOW window = new(content) { Owner = owner_ };
        window.ShowDialog();
        return window.chose_primary;
    }
}
