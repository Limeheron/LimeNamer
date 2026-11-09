using System.Collections.Generic;
using System.Text;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class MOD_JSON_TESTS
{
    [TestMethod]
    public void writes_back_what_the_engine_stores()
    {
        StringBuilder failures = new();
        foreach (object[] item in SUMMARY_CASES.load()) {
            if (MOD_JSON.parse((string)item[1]).direct) {
                continue;
            }
            using ENGINE engine = new();
            engine.mod_add((string)item[1]);
            string before = engine.mods_get();
            MOD stored = MOD_JSON.parse_list(before)[0];

            engine.mod_update(stored.id!.Value, MOD_JSON.write(stored));

            string after = engine.mods_get();
            if (after != before) {
                failures.AppendLine($"{item[0]}: {before} -> {after}");
            }
        }

        Assert.AreEqual("", failures.ToString());
    }

    [TestMethod]
    public void reads_engine_fields()
    {
        using ENGINE engine = new();
        engine.mod_add("""
            {"kind": "insert", "position": {"type": "end"},
                "value": [{"text": "_"}, {"token": "number"}],
                "tokens": {"number": {"start": 5, "step": 2, "digits": 3}}}
            """);

        IReadOnlyList<MOD> mods = MOD_JSON.parse_list(engine.mods_get());

        MOD mod = mods[0];
        Assert.AreEqual(1, mods.Count);
        Assert.IsTrue(mod.enabled);
        Assert.AreEqual(MOD_KIND.insert, mod.kind);
        Assert.AreEqual(PART.name, mod.part);
        Assert.AreEqual(POSITION_TYPE.end, mod.position.type);
        Assert.AreEqual("_", mod.value![0].text);
        Assert.AreEqual(TOKEN_KIND.number, mod.value[1].token);
        Assert.AreEqual(new NUMBER_SETTINGS { start = 5, step = 2, digits = 3 },
            mod.tokens!.number);
        Assert.AreEqual("yyyyMMdd", mod.tokens.modified_date!.format);
        Assert.AreEqual(SIZE_UNIT.auto, mod.tokens.size!.unit);
        Assert.AreEqual(SCOPE.all, mod.scope);
        Assert.IsFalse(mod.direct);
    }
}
