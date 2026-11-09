using System.Collections.Generic;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace limenamer;

public static class MOD_JSON
{
    private static readonly JsonSerializerOptions OPTIONS = new()
    {
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
        Converters = { new JsonStringEnumConverter(null, false) },
    };

    public static MOD parse(string json)
    {
        return JsonSerializer.Deserialize<MOD>(json, OPTIONS)
            ?? throw new JsonException("Modification JSON is null");
    }

    public static IReadOnlyList<MOD> parse_list(string json)
    {
        return JsonSerializer.Deserialize<List<MOD>>(json, OPTIONS)
            ?? throw new JsonException("Modification list JSON is null");
    }

    public static string write(MOD mod)
    {
        return JsonSerializer.Serialize(mod, OPTIONS);
    }
}
