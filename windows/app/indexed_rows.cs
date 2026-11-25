using System;
using System.Collections;
using System.Collections.Generic;

namespace limenamer;

public sealed class INDEXED_ROWS : IList, IReadOnlyList<ITEM_ROW>
{
    private readonly IReadOnlyList<ITEM_ROW> rows_;
    private readonly Dictionary<ITEM_ROW, int> indexes_;

    public INDEXED_ROWS(IReadOnlyList<ITEM_ROW> rows)
    {
        rows_ = rows;
        indexes_ = new Dictionary<ITEM_ROW, int>(rows.Count, ReferenceEqualityComparer.Instance);
        for (int index = 0; index < rows.Count; index++) {
            indexes_[rows[index]] = index;
        }
    }

    public int Count
    {
        get { return rows_.Count; }
    }

    public bool IsFixedSize
    {
        get { return true; }
    }

    public bool IsReadOnly
    {
        get { return true; }
    }

    public bool IsSynchronized
    {
        get { return false; }
    }

    public object SyncRoot
    {
        get { return this; }
    }

    public ITEM_ROW this[int index]
    {
        get { return rows_[index]; }
    }

    object? IList.this[int index]
    {
        get { return rows_[index]; }
        set { throw new NotSupportedException(); }
    }

    public int IndexOf(object? value)
    {
        return value is ITEM_ROW row && indexes_.TryGetValue(row, out int index) ? index : -1;
    }

    public bool Contains(object? value)
    {
        return IndexOf(value) >= 0;
    }

    public void CopyTo(Array array, int index)
    {
        for (int offset = 0; offset < rows_.Count; offset++) {
            array.SetValue(rows_[offset], index + offset);
        }
    }

    public IEnumerator<ITEM_ROW> GetEnumerator()
    {
        return rows_.GetEnumerator();
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
        return rows_.GetEnumerator();
    }

    public int Add(object? value)
    {
        throw new NotSupportedException();
    }

    public void Clear()
    {
        throw new NotSupportedException();
    }

    public void Insert(int index, object? value)
    {
        throw new NotSupportedException();
    }

    public void Remove(object? value)
    {
        throw new NotSupportedException();
    }

    public void RemoveAt(int index)
    {
        throw new NotSupportedException();
    }
}
