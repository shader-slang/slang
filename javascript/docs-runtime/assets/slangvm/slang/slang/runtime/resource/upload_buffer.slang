// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

__target_intrinsic(hgbc, "OpUploadBufferStorageType") struct UploadBufferStorage
{
}

public struct UploadBuffer<T>
{
    private UploadBufferStorage storage;
    private uint count;

    private __init(UploadBufferStorage storage, uint count)
    {
        this.storage = storage;
        this.count = count;
    }

    public static UploadBuffer<T> allocate(uint count)
    {
        uint one = 1;
        UploadBufferStorage storage = hgbc_asm
        {
        result:
            $$UploadBufferStorage = OpAllocateResource Buffer Buffer Undefined Upload CopySource $$T
                $count $one $one 1 1 0 CopySource;
        };
        return UploadBuffer<T>(storage, count);
    }

    public uint getCount() { return count; }

    public void store(uint index, T value)
    {
        hgbc_asm
        {
            OpUploadBufferStore $$T $storage $index $value;
        };
    }

    public __subscript(uint index)->T
    {
        [nonmutating]
        set
        {
            store(index, newValue);
        }
    }

    public void uploadTo(RWStructuredBuffer<T> destination)
    {
        hgbc_asm
        {
            OpUploadBufferUploadTo $storage $destination;
        };
    }

    public void uploadTo(StructuredBuffer<T> destination)
    {
        hgbc_asm
        {
            OpUploadBufferUploadTo $storage $destination;
        };
    }
}
