//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

String format(String pattern, String value)
{
    return hgbc_asm {
        result : $$String = OpFormatString $pattern $$String $value;
    };
}

__target_intrinsic(hgbc, "OpByteStreamType")
struct ByteStream {}

ByteStream openByteStream(String path, uint mode)
{
    if (mode == 0)
    {
        return hgbc_asm {
            result : $$ByteStream = OpByteStreamFromFile Read $path;
        };
    }
    return hgbc_asm {
        result : $$ByteStream = OpByteStreamFromFile Write $path;
    };
}

void closeByteStream(ByteStream stream)
{
    hgbc_asm {
        OpCloseByteStream $stream;
    };
}

uint readValue<T>(ByteStream stream, out T destination)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            HostAddress $stream &destination $byteOffset $byteCount;
    };
}

uint writeValue<T>(ByteStream stream, __constref T source)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamWrite
            HostAddress $stream &source $byteOffset $byteCount;
    };
}

[host]
void main(String baseFolder)
{
    String path = format("{}/data.bin", baseFolder);
    ByteStream input = openByteStream(path, 0);
    uint count;
    uint countBytes = readValue(input, count);
    uint zero = 0;
    RWStructuredBuffer<float> buffer = RWStructuredBuffer<float>::alloc(count);
    uint payloadSize = count * sizeof(float);
    uint bytesRead = hgbc_asm {
        result : $$uint = OpByteStreamRead
            DeviceBuffer $input $buffer $zero $payloadSize;
    };
    closeByteStream(input);
    ByteStream output = openByteStream(path, 1);
    uint bytesWritten = writeValue(output, count);
    uint payloadBytes = hgbc_asm {
        result : $$uint = OpByteStreamWrite
            DeviceBuffer $output $buffer $zero $payloadSize;
    };
    closeByteStream(output);
}

// Helper calls may remain as separate HGBC functions, so check the required operations without
// requiring them to be inlined into the host entry point.
// CHECK-DAG: OpFormatString
// CHECK-DAG: OpByteStreamFromFile Read
// CHECK-DAG: OpByteStreamFromFile Write
// CHECK-DAG: OpByteStreamRead HostAddress
// CHECK-DAG: OpAllocateResource Buffer Buffer Undefined DeviceLocal UnorderedAccess
// CHECK-DAG: OpByteStreamRead DeviceBuffer
// CHECK-DAG: OpCloseByteStream
// CHECK-DAG: OpByteStreamWrite
