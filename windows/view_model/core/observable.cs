using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace limenamer;

public abstract class OBSERVABLE : INotifyPropertyChanged
{
    public event PropertyChangedEventHandler? PropertyChanged;

    protected bool assign<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value)) {
            return false;
        }
        field = value;
        notify(name);
        return true;
    }

    protected void notify(string? name)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
