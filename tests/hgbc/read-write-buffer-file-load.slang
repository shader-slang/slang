//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

__target_intrinsic(hgbc, "OpByteStreamType")
struct ByteStream {}

ByteStream openByteStream(String path)
{
    return hgbc_asm {
        result : $$ByteStream = OpByteStreamFromFile Read $path;
    };
}

void closeByteStream(ByteStream stream)
{
    hgbc_asm {
        OpCloseByteStream $stream;
    };
}

uint readValue<T>(ByteStream stream, out T result)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            HostAddress $stream &result $byteOffset $byteCount;
    };
}

uint readUInt32LE(ByteStream stream)
{
    uint result;
    readValue(stream, result);
    return result;
}

extension<T> RWStructuredBuffer<T>
{
    static RWStructuredBuffer<T> loadBuffer(String path)
    {
        ByteStream stream = openByteStream(path);
        uint count = readUInt32LE(stream);
        RWStructuredBuffer<T> result = RWStructuredBuffer<T>::alloc(count);
        uint byteOffset = 0;
        uint byteCount = count * sizeof(T);
        uint bytesRead = hgbc_asm {
            result : $$uint = OpByteStreamRead
                DeviceBuffer $stream $result $byteOffset $byteCount;
        };
        closeByteStream(stream);
        return result;
    }
}

[host]
RWStructuredBuffer<uint> main(String path)
{
    return RWStructuredBuffer<uint>::loadBuffer(path);
}

// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[UINT]]
// Helper calls may remain as separate HGBC functions.
// CHECK-DAG: OpByteStreamFromFile Read
// CHECK-DAG: OpByteStreamRead HostAddress
// CHECK-DAG: OpAllocateResource Buffer Buffer Undefined DeviceLocal UnorderedAccess
// CHECK-DAG: OpByteStreamRead DeviceBuffer
// CHECK-DAG: OpCloseByteStream
