using System;
using System.Windows.Input;

namespace limenamer;

public sealed class COMMAND : ICommand
{
    private readonly Action<object?> execute_;
    private readonly Func<object?, bool>? can_execute_;

    public COMMAND(Action<object?> execute, Func<object?, bool>? can_execute = null)
    {
        execute_ = execute;
        can_execute_ = can_execute;
    }

    public event EventHandler? CanExecuteChanged;

    public bool CanExecute(object? parameter)
    {
        return can_execute_ == null || can_execute_(parameter);
    }

    public void Execute(object? parameter)
    {
        if (CanExecute(parameter)) {
            execute_(parameter);
        }
    }

    public void refresh()
    {
        CanExecuteChanged?.Invoke(this, EventArgs.Empty);
    }
}
