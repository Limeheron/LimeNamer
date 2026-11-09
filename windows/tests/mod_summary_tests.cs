using System.Collections.Generic;
using System.Text;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class MOD_SUMMARY_TESTS
{
    public static IEnumerable<object[]> cases()
    {
        return SUMMARY_CASES.load();
    }

    [TestMethod]
    [DynamicData(nameof(cases))]
    public void describes_mods(string name, string mod_json, uint direct_index, uint target_count,
        string expected)
    {
        MOD mod = MOD_JSON.parse(mod_json);

        Assert.AreEqual(expected, MOD_SUMMARY.describe(mod, direct_index, target_count), name);
    }

    [TestMethod]
    public void describes_mods_read_back_from_engine()
    {
        StringBuilder failures = new();
        foreach (object[] item in SUMMARY_CASES.load()) {
            MOD mod = MOD_JSON.parse((string)item[1]);
            if (mod.direct || mod.scope != SCOPE.all) {
                continue;
            }
            using ENGINE engine = new();
            engine.mod_add((string)item[1]);
            MOD stored = MOD_JSON.parse_list(engine.mods_get())[0];
            string summary = MOD_SUMMARY.describe(stored, 0, 0);
            if (summary != (string)item[4]) {
                failures.AppendLine($"{item[0]}: {summary}");
            }
        }

        Assert.AreEqual("", failures.ToString());
    }
}
