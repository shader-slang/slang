//TEST:INTERPRET(hgvm): -api default

__target_intrinsic(hgbc, "OpByteStreamType")
struct ByteStream
{
}

/// Opens one forward-only input stream. The enum is embedded in HGBC rather than inspected from a
/// runtime mode value.
ByteStream openInput(String path)
{
    return hgbc_asm {
        result : $$ByteStream = OpByteStreamFromFile Read $path;
    };
}

/// Opens one forward-only output stream.
ByteStream openOutput(String path)
{
    return hgbc_asm {
        result : $$ByteStream = OpByteStreamFromFile Write $path;
    };
}

void closeStream(ByteStream stream)
{
    hgbc_asm {
        OpCloseByteStream $stream;
    };
}

/// Reads one arbitrary value representation into a caller-owned stack slot.
///
/// For example, `Header` below is transferred as its staged eight-byte representation without a
/// scalar-specific bytecode or runtime type lookup.
uint readValue<T>(ByteStream stream, out T destination)
{
    destination = T();
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            HostAddress $stream &destination $byteOffset $byteCount;
    };
}

/// Writes one arbitrary stack value through the same raw endpoint model used by reads.
uint writeValue<T>(ByteStream stream, __constref T source)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm {
        result : $$uint = OpByteStreamWrite
            HostAddress $stream &source $byteOffset $byteCount;
    };
}

uint readBuffer(
    ByteStream stream,
    RWStructuredBuffer<uint> destination,
    uint byteCount)
{
    uint byteOffset = 0;
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            DeviceBuffer $stream $destination $byteOffset $byteCount;
    };
}

uint writeBuffer(
    ByteStream stream,
    RWStructuredBuffer<uint> source,
    uint byteCount)
{
    uint byteOffset = 0;
    return hgbc_asm {
        result : $$uint = OpByteStreamWrite
            DeviceBuffer $stream $source $byteOffset $byteCount;
    };
}

uint readTexture(
    ByteStream stream,
    Texture2D<float4, 0, 10> destination,
    uint byteCount)
{
    uint byteOffset = 0;
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            DeviceTexture2D $stream $destination $byteOffset $byteCount;
    };
}

Texture2D<float4, 0, 10> allocateRGBA8Texture(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm {
        result : $$Texture2D<float4, 0, 10> = OpAllocateResource
            Texture Texture2D 10 DeviceLocal ShaderResource $$float4
            $width $height $one 1 1 0
            ShaderResource CopySource CopyDestination;
    };
}

RWStructuredBuffer<uint> allocateUIntBuffer(uint count)
{
    uint one = 1;
    return hgbc_asm {
        result : $$RWStructuredBuffer<uint> = OpAllocateResource
            Buffer Buffer Undefined DeviceLocal UnorderedAccess $$uint
            $count $one $one 1 1 0
            ShaderResource UnorderedAccess CopySource CopyDestination;
    };
}

uint readBack(RWStructuredBuffer<uint> buffer, uint index)
{
    return hgbc_asm {
        result : $$uint = OpReadBackBufferElement $$RWStructuredBuffer<uint> $buffer $index;
    };
}

void fail(String message)
{
    hgbc_asm {
        OpFail $message;
    };
}

struct Header
{
    uint tag;
    float scale;
}

struct PackedFlags
{
    Array<bool, 3> flags;
    uint value;
}

uint readPackedFlagsBuffer(
    ByteStream stream,
    RWStructuredBuffer<PackedFlags> destination,
    uint byteCount)
{
    uint byteOffset = 0;
    return hgbc_asm {
        result : $$uint = OpByteStreamRead
            DeviceBuffer $stream $destination $byteOffset $byteCount;
    };
}

PackedFlags readPackedFlags(RWStructuredBuffer<PackedFlags> buffer, uint index)
{
    return hgbc_asm {
        result : $$PackedFlags = OpReadBackBufferElement
            $$RWStructuredBuffer<PackedFlags> $buffer $index;
    };
}

[shader("compute")]
[numthreads(4, 1, 1)]
void initializePayload(
    RWStructuredBuffer<uint> payload,
    uint3 threadID : SV_DispatchThreadID)
{
    payload[threadID.x] = 10 + threadID.x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void verifyPayload(
    RWStructuredBuffer<uint> payload,
    RWStructuredBuffer<uint> result)
{
    result[0] = payload[0] == 10 && payload[1] == 11 && payload[2] == 12 && payload[3] == 13;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void verifyTexture(Texture2D<float4, 0, 10> texture, RWStructuredBuffer<uint> result)
{
    uint4 red = uint4(texture[uint2(0, 0)] * 255.0f + 0.5f);
    uint4 green = uint4(texture[uint2(1, 0)] * 255.0f + 0.5f);
    uint4 blue = uint4(texture[uint2(0, 1)] * 255.0f + 0.5f);
    uint4 white = uint4(texture[uint2(1, 1)] * 255.0f + 0.5f);
    result[0] = all(red == uint4(255, 0, 0, 255)) &&
                all(green == uint4(0, 255, 0, 255)) &&
                all(blue == uint4(0, 0, 255, 255)) &&
                all(white == uint4(255, 255, 255, 255));
}

[host]
void main()
{
    Header expectedHeader = {0x12345678, 1.25};
    RWStructuredBuffer<uint> source = allocateUIntBuffer(4);
    dispatch(initializePayload, uint3(1, 1, 1))(source);

    ByteStream output = openOutput("byte-stream.actual");
    uint headerBytesWritten = writeValue(output, expectedHeader);
    uint payloadBytesWritten = writeBuffer(output, source, 4 * sizeof(uint));
    closeStream(output);

    ByteStream input = openInput("byte-stream.actual");
    Header actualHeader;
    uint headerBytesRead = readValue(input, actualHeader);
    RWStructuredBuffer<uint> destination = allocateUIntBuffer(4);
    uint payloadBytesRead = readBuffer(input, destination, 4 * sizeof(uint));
    closeStream(input);

    // Read back before `destination` is ever bound to a shader. The converter operand on the
    // readback operation must resolve its device and host layouts independently; relying on a
    // previous compute binding to populate buffer metadata would fail here.
    if (readBack(destination, 2) != 12)
        fail("Buffer readback unexpectedly depends on an earlier shader binding");

    // These uints are the structured-buffer representation of three booleans and one uint. This
    // buffer is populated by a raw stream transfer and is deliberately never bound to a shader.
    // Its readback converter must still compact the four-byte device bools into host bool slots.
    Array<uint, 4> packedDeviceBytes = {1, 0, 1, 99};
    ByteStream aggregateOutput = openOutput("byte-stream-aggregate.actual");
    writeValue(aggregateOutput, packedDeviceBytes);
    closeStream(aggregateOutput);

    RWStructuredBuffer<PackedFlags> packedBuffer =
        RWStructuredBuffer<PackedFlags>::alloc(1);
    ByteStream aggregateInput = openInput("byte-stream-aggregate.actual");
    readPackedFlagsBuffer(aggregateInput, packedBuffer, 4 * sizeof(uint));
    closeStream(aggregateInput);
    PackedFlags packed = readPackedFlags(packedBuffer, 0);
    if (!packed.flags[0] || packed.flags[1] || !packed.flags[2] || packed.value != 99)
        fail("Aggregate readback did not apply its independent layout converter");

    if (headerBytesWritten != sizeof(Header) || headerBytesRead != sizeof(Header) ||
        payloadBytesWritten != 4 * sizeof(uint) || payloadBytesRead != 4 * sizeof(uint) ||
        actualHeader.tag != expectedHeader.tag || actualHeader.scale != expectedHeader.scale)
    {
        fail("Raw host byte-stream round trip failed");
    }

    RWStructuredBuffer<uint> result = allocateUIntBuffer(1);
    dispatch(verifyPayload, uint3(1, 1, 1))(destination, result);
    if (readBack(result, 0) != 1)
        fail("Raw device-buffer byte-stream round trip failed");

    Array<uint, 4> packedTexels = {
        0xff0000ff,
        0xff00ff00,
        0xffff0000,
        0xffffffff,
    };
    ByteStream textureOutput = openOutput("byte-stream-texture.actual");
    writeValue(textureOutput, packedTexels);
    closeStream(textureOutput);

    Texture2D<float4, 0, 10> texture = allocateRGBA8Texture(2, 2);
    ByteStream textureInput = openInput("byte-stream-texture.actual");
    if (readTexture(textureInput, texture, 16) != 16)
        fail("Raw texture byte-stream upload returned the wrong byte count");
    closeStream(textureInput);
    dispatch(verifyTexture, uint3(1, 1, 1))(texture, result);
    if (readBack(result, 0) != 1)
        fail("Raw device-texture byte-stream upload failed");
}
