//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

__target_intrinsic(hgbc, "OpByteStreamType")
struct ByteStream
{
}

ByteStream openHalfBufferStream(String path, bool write)
{
    if (write)
    {
        return hgbc_asm {
            result : $$ByteStream = OpByteStreamFromFile Write $path;
        };
    }
    return hgbc_asm {
        result : $$ByteStream = OpByteStreamFromFile Read $path;
    };
}

void closeHalfBufferStream(ByteStream stream)
{
    hgbc_asm {
        OpCloseByteStream $stream;
    };
}

uint writeHalfBuffer(ByteStream stream, RWStructuredBuffer<half2> source)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(half2);
    return hgbc_asm {
        result : $$uint = OpByteStreamWrite
            DeviceBuffer $stream $source $byteOffset $byteCount;
    };
}

uint readHalfBuffer(ByteStream stream, StructuredBuffer<half2> destination)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(half2);
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            DeviceBuffer $stream $destination $byteOffset $byteCount;
    };
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeHalfPair(RWStructuredBuffer<half2> values)
{
    values[0] = half2(1.5h, -2.25h);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void widenHalfPair(StructuredBuffer<half2> values, RWStructuredBuffer<float> result)
{
    result[0] = float(values[0].x);
    result[1] = float(values[0].y);
}

[host]
void main()
{
    // Inline literals prevent constant folding so HGVM executes the binary16 operations.
    half onePointFive = hgbc_asm { result : $$half = OpFloatLit 15872; };
    half two = hgbc_asm { result : $$half = OpFloatLit 16384; };
    uint three = hgbc_asm { result : $$uint = OpIntLit 3; };
    half result = onePointFive * two + 0.25h;
    print(result);
    print(onePointFive < two);
    print(float(-onePointFive));
    print(half(three));
    print(int(onePointFive));
    print(bool(onePointFive));
    print(two / onePointFive);

    RWStructuredBuffer<half2> values = RWStructuredBuffer<half2>::alloc(1);
    dispatch(writeHalfPair, uint3(1, 1, 1))(values);
    half2 pair = readBack(values, 0);
    print(float(pair.x));
    print(float(pair.y));

    ByteStream output = openHalfBufferStream("half-buffer.actual", true);
    if (writeHalfBuffer(output, values) != sizeof(half2))
        return;
    closeHalfBufferStream(output);

    StructuredBuffer<half2> uploadedValues = StructuredBuffer<half2>::alloc(1);
    ByteStream input = openHalfBufferStream("half-buffer.actual", false);
    if (readHalfBuffer(input, uploadedValues) != sizeof(half2))
        return;
    closeHalfBufferStream(input);

    RWStructuredBuffer<float> widenedValues = RWStructuredBuffer<float>::alloc(2);
    dispatch(widenHalfPair, uint3(1, 1, 1))(uploadedValues, widenedValues);
    print(readBack(widenedValues, 0));
    print(readBack(widenedValues, 1));
}

// CHECK: 3.250000
// CHECK-NEXT: true
// CHECK-NEXT: -1.500000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 1
// CHECK-NEXT: true
// CHECK-NEXT: 1.333008
// CHECK-NEXT: 1.500000
// CHECK-NEXT: -2.250000
// CHECK-NEXT: 1.500000
// CHECK-NEXT: -2.250000
