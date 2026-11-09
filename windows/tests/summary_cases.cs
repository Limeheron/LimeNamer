using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace limenamer;

internal static class SUMMARY_CASES
{
    internal static IEnumerable<object[]> load()
    {
        string path = Path.Combine(AppContext.BaseDirectory, "testdata", "mod_summary_cases.json");
        using JsonDocument document = JsonDocument.Parse(File.ReadAllText(path));
        foreach (JsonElement item in document.RootElement.EnumerateArray()) {
            yield return [
                item.GetProperty("name").GetString()!,
                item.GetProperty("mod").GetRawText(),
                read_count(item, "direct_index"),
                read_count(item, "target_count"),
                item.GetProperty("expected").GetString()!,
            ];
        }
    }

    private static uint read_count(JsonElement item, string name)
    {
        return item.TryGetProperty(name, out JsonElement value) ? value.GetUInt32() : 0;
    }
}
