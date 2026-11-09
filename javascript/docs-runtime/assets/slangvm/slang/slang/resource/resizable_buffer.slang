import slang.runtime.core.simple_string;

public struct ResizeableBuffer<T>
{
    /// Captures one allocation and its capacity as shader data. Host copies retain its descriptor.
    public struct Handle
    {
        public RWStructuredBuffer<T>.Handle buffer;
        public uint capacity;
    }

    public RWStructuredBuffer<T> buffer;
    public uint capacity;

    public __init(uint count)
    {
        capacity = count == 0 ? 1 : count;
        buffer = RWStructuredBuffer<T>::alloc(capacity);
    }

    /// Reconstructs a view of an existing allocation without allocating or resizing storage.
    public __init(Handle handle)
    {
        buffer = *handle.buffer;
        capacity = handle.capacity;
    }

    /// Snapshots the current allocation; later reserve() calls do not change this handle.
    [host]
    public Handle getDescriptorHandle() { return Handle(buffer.getDescriptorHandle(), capacity); }

    [mutating]
    public void reserve(uint count)
    {
        if (count <= capacity)
            return;
        buffer = RWStructuredBuffer<T>::alloc(count);
        capacity = count;
    }
}

__generic<T> [ForceInline]
public __prefix ResizeableBuffer<T> operator*(ResizeableBuffer<T>.Handle handle)
{
    return ResizeableBuffer<T>(handle);
}
