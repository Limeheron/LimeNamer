using System.Linq;
using System.Windows;

namespace limenamer;

public sealed class MESSAGE_BOX_DIALOGS : DIALOG_SERVICE
{
    private readonly Window owner_;

    public MESSAGE_BOX_DIALOGS(Window owner)
    {
        owner_ = owner;
    }

    public bool ask(DIALOG_CONTENT content)
    {
        string lines = string.Join("\n", content.lines.Select(line =>
            $"{line.index}  {line.text}  {line.reason}"));
        string text = string.Join("\n\n",
            new[] { content.message, lines, content.note }.Where(part => part != ""));
        MessageBoxButton buttons = content.secondary == ""
            ? MessageBoxButton.OK
            : MessageBoxButton.OKCancel;
        MessageBoxResult result = MessageBox.Show(owner_, text, content.title, buttons);
        return result == MessageBoxResult.OK;
    }
}
