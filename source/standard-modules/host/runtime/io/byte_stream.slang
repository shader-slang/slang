// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

__target_intrinsic(hgbc, "OpByteStreamType") public struct ByteStream
{
}

public enum ByteStreamMode : uint
{
    read = 0,
    write = 1,
}

public ByteStream openByteStream(String path, ByteStreamMode mode)
{
    if (mode == ByteStreamMode::read)
    {
        return hgbc_asm
        {
        result:
            $$ByteStream = OpByteStreamFromFile Read $path;
        };
    }
    return hgbc_asm
    {
    result:
        $$ByteStream = OpByteStreamFromFile Write $path;
    };
}

public void closeByteStream(ByteStream stream)
{
    hgbc_asm
    {
        OpCloseByteStream $stream;
    };
}

public uint getByteStreamSize(ByteStream stream)
{
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamGetSize $stream;
    };
}

/// Reads raw sequential bytes into an addressable host value.
///
/// For example, reading four bytes into `out uint` writes directly into that variable's backing
/// stack slot. The HGBC validator rejects managed values whose bytes cannot safely come from a
/// file or network stream.
public uint readByteStreamBytes<T>(
    ByteStream stream,
    inout T destination,
    uint destinationByteOffset,
    uint byteCount)
{
    return hgbc_asm
    {
    result:
        $$uint =
            OpByteStreamRead HostAddress $stream & destination $destinationByteOffset $byteCount;
    };
}

public uint readByteStream<T>(ByteStream stream, out T destination)
{
    // `hgbc_asm` writes through an address, which definite-assignment analysis cannot observe.
    // Initializing here also gives bytes outside a future partial read a deterministic value.
    destination = T();
    return readByteStreamBytes(stream, destination, 0, sizeof(T));
}

/// Reads one scalar-layout POD value from a little-endian stream.
///
/// HGVM currently runs on little-endian hosts, so the serialized and host byte orders agree. The
/// raw host-address endpoint rejects values that contain managed state.
public extension ByteStream
{
    public T read<T>()
    {
        T result;
        readByteStream(this, result);
        return result;
    }
}

public uint readByte(ByteStream stream)
{
    uint result = 0;
    readByteStreamBytes(stream, result, 0, 1);
    return result;
}

public uint readByteStreamBuffer<T, L : IBufferDataLayout>(
    ByteStream stream,
    RWStructuredBuffer<T, L> destination)
{
    uint elementCount;
    uint elementStride;
    destination.GetDimensions(elementCount, elementStride);
    uint byteOffset = 0;
    uint byteCount = elementCount * elementStride;
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamRead DeviceBuffer $stream $destination $byteOffset $byteCount;
    };
}

/// Reads one complete tightly packed texture payload directly into device storage.
///
/// The byte representation is the texture's physical allocation format, not its logical shader
/// element type. Backends that cannot apply format conversion to writable texture access may reject
/// a physical format whose texel width differs from the shader element width.
public uint readByteStreamTexture2D<T : ITexelElement, let format : int>(
    ByteStream stream,
    RWTexture2D<T, 0, format> destination,
    uint byteCount)
{
    uint byteOffset = 0;
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamRead DeviceTexture2D $stream $destination $byteOffset $byteCount;
    };
}

/// Uploads tightly packed physical texels without granting shaders writable access. For example,
/// an RGBA8Unorm-backed `Texture2D<float4>` consumes four bytes per texel and exposes normalized
/// floats when subsequently read by a shader.
public uint readByteStreamTexture2D<T : ITexelElement, let format : int>(
    ByteStream stream,
    Texture2D<T, 0, format> destination,
    uint byteCount)
{
    uint byteOffset = 0;
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamRead DeviceTexture2D $stream $destination $byteOffset $byteCount;
    };
}

public uint readByteStreamBuffer<T, L : IBufferDataLayout>(
    ByteStream stream,
    StructuredBuffer<T, L> destination)
{
    uint elementCount;
    uint elementStride;
    destination.GetDimensions(elementCount, elementStride);
    uint byteOffset = 0;
    uint byteCount = elementCount * elementStride;
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamRead DeviceBuffer $stream $destination $byteOffset $byteCount;
    };
}

/// Reads raw sequential bytes directly into a reference-counted host array.
///
/// This endpoint is useful for large host-side records such as binary PLY vertices: one HGBC
/// operation fills the complete allocation instead of staging one stream read per field.
[require(hgbc)]
public uint readByteStreamArrayBytes<T>(
    ByteStream stream,
    RuntimeArray<T> destination,
    uint destinationByteOffset,
    uint byteCount)
{
    return hgbc_asm
    {
    result:
        $$uint =
            OpByteStreamRead RuntimeArray $stream $destination $destinationByteOffset $byteCount;
    };
}

/// Reads exactly the byte representation of every element in a runtime array.
[require(hgbc)]
public uint readByteStreamArray<T>(ByteStream stream, RuntimeArray<T> destination)
{
    uint byteCount = uint(destination.getCount()) * sizeof(T);
    return readByteStreamArrayBytes(stream, destination, 0, byteCount);
}

public uint writeByteStream<T>(ByteStream stream, __constref T value)
{
    uint byteOffset = 0;
    uint byteCount = sizeof(T);
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamWrite HostAddress $stream & value $byteOffset $byteCount;
    };
}

/// Writes one scalar-layout POD value to a little-endian stream.
public extension ByteStream
{
    public uint write<T>(__constref T value)
    {
        return writeByteStream(this, value);
    }
}

/// Writes the complete contents of a read-write structured buffer to a sequential byte stream.
///
/// For example, a four-element `RWStructuredBuffer<float2>` writes the four elements using the
/// buffer's device layout. Naming this separately from `writeByteStream` also prevents a resource
/// handle from matching the generic host-value overload.
public uint writeByteStreamBuffer<T>(ByteStream stream, RWStructuredBuffer<T> value)
{
    uint elementCount;
    uint elementStride;
    value.GetDimensions(elementCount, elementStride);
    uint byteOffset = 0;
    uint byteCount = elementCount * elementStride;
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamWrite DeviceBuffer $stream $value $byteOffset $byteCount;
    };
}

/// Writes the complete contents of a read-only structured buffer to a sequential byte stream.
public uint writeByteStreamBuffer<T>(ByteStream stream, StructuredBuffer<T> value)
{
    uint elementCount;
    uint elementStride;
    value.GetDimensions(elementCount, elementStride);
    uint byteOffset = 0;
    uint byteCount = elementCount * elementStride;
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamWrite DeviceBuffer $stream $value $byteOffset $byteCount;
    };
}

/// Writes raw bytes from a reference-counted host array to a sequential stream.
[require(hgbc)]
public uint writeByteStreamArrayBytes<T>(
    ByteStream stream,
    RuntimeArray<T> source,
    uint sourceByteOffset,
    uint byteCount)
{
    return hgbc_asm
    {
    result:
        $$uint = OpByteStreamWrite RuntimeArray $stream $source $sourceByteOffset $byteCount;
    };
}

/// Writes exactly the byte representation of every element in a runtime array.
[require(hgbc)]
public uint writeByteStreamArray<T>(ByteStream stream, RuntimeArray<T> source)
{
    uint byteCount = uint(source.getCount()) * sizeof(T);
    return writeByteStreamArrayBytes(stream, source, 0, byteCount);
}

public uint getStringByteCount(String value)
{
    return hgbc_asm
    {
    result:
        $$uint = OpStringGetByteCount $value;
    };
}

public uint readStringByte(String value, uint index)
{
    return hgbc_asm
    {
    result:
        $$uint = OpStringReadByte $value $index;
    };
}
