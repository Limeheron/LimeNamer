using System.Collections.Generic;
using System.Windows.Controls;

namespace limenamer;

public sealed class ITEM_GRID : DataGrid
{
    public void select_items(IReadOnlyCollection<object> items)
    {
        if (items.Count > 0 && items.Count == Items.Count) {
            SelectAll();
            return;
        }
        BeginUpdateSelectedItems();
        try {
            SelectedItems.Clear();
            foreach (object item in items) {
                SelectedItems.Add(item);
            }
        } finally {
            EndUpdateSelectedItems();
        }
    }
}
