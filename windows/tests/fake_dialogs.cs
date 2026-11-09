using System.Collections.Generic;

namespace limenamer;

internal sealed class FAKE_DIALOGS : DIALOG_SERVICE
{
    public Queue<bool> answers { get; } = new();
    public List<DIALOG_CONTENT> asked { get; } = new();

    public bool ask(DIALOG_CONTENT content)
    {
        asked.Add(content);
        return answers.Count == 0 || answers.Dequeue();
    }
}
