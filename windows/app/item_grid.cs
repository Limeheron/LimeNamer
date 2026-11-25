using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;

namespace limenamer;

public sealed class ITEM_GRID : DataGrid
{
    public void select_all_items()
    {
        select_items(Items.Cast<object>().ToArray());
    }

    public void select_items(IReadOnlyCollection<object> items)
    {
        if (is_selected(items)) {
            return;
        }
        UnselectAllCells();
        if (items.Count == 0) {
            return;
        }
        BeginUpdateSelectedItems();
        try {
            foreach (object item in items) {
                SelectedItems.Add(item);
            }
        } finally {
            EndUpdateSelectedItems();
        }
    }

    private bool is_selected(IReadOnlyCollection<object> items)
    {
        if (SelectedItems.Count != items.Count) {
            return false;
        }
        HashSet<object> selected = new(SelectedItems.Cast<object>(),
            ReferenceEqualityComparer.Instance);
        return items.All(selected.Contains);
    }
}
