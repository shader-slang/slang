//TEST:SIMPLE(filecheck=CHECK): -target hgbc

__target_intrinsic(hgbc, "OpByteStreamType")
struct TestByteStream
{
}

__target_intrinsic(hgbc, "OpUploadBufferStorageType")
struct TestUploadBufferStorage
{
}

public struct TestVertex
{
    public float3 position;
    public uint tag;
}

uint readValue<T>(TestByteStream stream, out T destination)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            HostAddress $stream &destination $byteOffset $byteCount;
    };
}

uint writeValue<T>(TestByteStream stream, __constref T source)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamWrite
            HostAddress $stream &source $byteOffset $byteCount;
    };
}

[host]
public RWStructuredBuffer<TestVertex> buildBuffer(String path)
{
    TestByteStream stream = hgbc_asm {
        result : $$TestByteStream = OpByteStreamFromFile Read $path;
    };
    uint size = hgbc_asm {
        result : $$uint = OpByteStreamGetSize $stream;
    };
    uint tag;
    uint tagBytes = readValue(stream, tag);
    float value;
    uint valueBytes = readValue(stream, value);
    hgbc_asm {
        OpCloseByteStream $stream;
    };

    uint elementCount = 1;
    uint one = 1;
    TestUploadBufferStorage upload = hgbc_asm {
        result : $$TestUploadBufferStorage = OpAllocateResource
            Buffer Buffer Undefined Upload CopySource $$TestVertex
            $elementCount $one $one 1 1 0 CopySource;
    };
    TestVertex vertex;
    vertex.position = float3(value, float(size), 0.0f);
    vertex.tag = tag;
    uint elementIndex = 0;
    hgbc_asm {
        OpUploadBufferStore $$TestVertex $upload $elementIndex $vertex;
    };

    String outputPath = "layout.bin";
    TestByteStream output = hgbc_asm {
        result : $$TestByteStream = OpByteStreamFromFile Write $outputPath;
    };
    uint payloadBytes = writeValue(output, vertex);
    hgbc_asm {
        OpCloseByteStream $output;
    };

    RWStructuredBuffer<TestVertex> destination =
        RWStructuredBuffer<TestVertex>::alloc(elementCount);
    hgbc_asm {
        OpUploadBufferUploadTo $upload $destination;
    };
    return destination;
}

// CHECK-DAG: OpByteStreamType
// CHECK-DAG: OpUploadBufferStorageType
// CHECK-DAG: OpByteStreamFromFile Read
// CHECK-DAG: OpByteStreamFromFile Write
// CHECK-DAG: OpByteStreamGetSize
// CHECK-DAG: OpByteStreamRead HostAddress
// CHECK-DAG: OpCloseByteStream
// CHECK-DAG: OpAllocateResource Buffer Buffer Undefined Upload CopySource
// CHECK-DAG: OpUploadBufferStore
// CHECK-DAG: OpByteStreamWrite
// CHECK-DAG: OpUploadBufferUploadTo
