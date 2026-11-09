// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.core.simple_assert;

/// A half-open element interval: begin is included and end is excluded.
public struct Range
{
    public uint begin;
    public uint end;
}

public Range range(uint begin, uint end)
{
    assert(begin <= end);
    return Range(begin, end);
}

/// Retains one buffer allocation and an element interval without copying its contents.
/// Assignment copies the view. Use host() for a snapshot or copyFrom() to move elements.
public struct BufferView<T, Storage : IArray<T> = RWStructuredBuffer<T>>
{
    internal Storage storage;
    internal uint offset;
    internal uint count;

    [host]
    public __init(Storage buffer, Range selection)
    {
        assert(selection.begin <= selection.end);
        assert(selection.end <= uint(buffer.getCount()));
        storage = buffer;
        offset = selection.begin;
        count = selection.end - selection.begin;
    }

    public uint getCount() { return count; }
}

public extension<T, L : IBufferDataLayout> BufferView<T, RWStructuredBuffer<T, L>>
{
    /// Allocates an independent host array and waits for the selected GPU contents.
    [host]
    public RuntimeArray<T> host()
    {
        let result = RuntimeArray<T>::alloc(count);
        copy(result, 0, storage, offset, count);
        return result;
    }

    /// Uploads an equally sized host array into this view, retaining normal command ordering.
    [host]
    public void copyFrom(RuntimeArray<T> source)
    {
        assert(uint(source.getCount()) == count);
        copy(storage, offset, source, 0, count);
    }

    /// Copies equally sized device ranges. Physical layouts must match; overlap is disallowed.
    [host]
    public void copyFrom<LS : IBufferDataLayout>(BufferView<T, RWStructuredBuffer<T, LS>> source)
    {
        assert(source.count == count);
        copy(storage, offset, source.storage, source.offset, count);
    }

    [host]
    public void copyFrom<LS : IBufferDataLayout>(BufferView<T, StructuredBuffer<T, LS>> source)
    {
        assert(source.count == count);
        copy(storage, offset, source.storage, source.offset, count);
    }
}

public extension<T, L : IBufferDataLayout> BufferView<T, StructuredBuffer<T, L>>
{
    /// Allocates an independent host array and waits for the selected GPU contents.
    [host]
    public RuntimeArray<T> host()
    {
        let result = RuntimeArray<T>::alloc(count);
        copy(result, 0, storage, offset, count);
        return result;
    }
}

public extension<T, L : IBufferDataLayout> RWStructuredBuffer<T, L>
{
    public __subscript(Range selection) -> BufferView<T, RWStructuredBuffer<T, L>>
    {
        [host]
        get { return BufferView<T, RWStructuredBuffer<T, L>>(this, selection); }

        [host]
        [nonmutating]
        set
        {
            BufferView<T, RWStructuredBuffer<T, L>>(this, selection).copyFrom(newValue);
        }
    }
}

public extension<T, L : IBufferDataLayout> StructuredBuffer<T, L>
{
    public __subscript(Range selection) -> BufferView<T, StructuredBuffer<T, L>>
    {
        [host]
        get { return BufferView<T, StructuredBuffer<T, L>>(this, selection); }
    }
}
