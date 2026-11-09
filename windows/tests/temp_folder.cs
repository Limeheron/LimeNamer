using System;
using System.IO;

namespace limenamer;

internal sealed class TEMP_FOLDER : IDisposable
{
    public TEMP_FOLDER()
    {
        path = Path.Combine(Path.GetTempPath(), "limenamer_tests_" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(path);
    }

    public string path { get; }

    public string file(string name, string content = "")
    {
        string full_path = Path.Combine(path, name);
        File.WriteAllText(full_path, content);
        return full_path;
    }

    public string folder(string name)
    {
        return Directory.CreateDirectory(Path.Combine(path, name)).FullName;
    }

    public bool exists(string name)
    {
        return Path.Exists(Path.Combine(path, name));
    }

    public void Dispose()
    {
        Directory.Delete(path, true);
    }
}
