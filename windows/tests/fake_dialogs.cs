using System;
using System.Collections.Generic;

namespace limenamer;

internal sealed class FAKE_DIALOGS : DIALOG_SERVICE
{
    public Queue<bool> answers { get; } = new();
    public List<DIALOG_CONTENT> asked { get; } = new();
    public Action<DIALOG_CONTENT>? on_ask { get; set; }

    public bool ask(DIALOG_CONTENT content)
    {
        asked.Add(content);
        on_ask?.Invoke(content);
        return answers.Count == 0 || answers.Dequeue();
    }
}
