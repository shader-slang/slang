// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.io.byte_stream;
import slang.runtime.core.simple_assert;

// Allocates a read-write resource and fills it from a count-prefixed structured-buffer file.
public extension<T> RWStructuredBuffer<T>
{
    public static RWStructuredBuffer<T> loadBuffer(String path)
    {
        ByteStream stream = openByteStream(path, ByteStreamMode.read);
        uint count = stream.read<uint>();
        RWStructuredBuffer<T> result = RWStructuredBuffer<T>::alloc(count);
        uint bytesRead = readByteStreamBuffer(stream, result);
        uint streamSize = getByteStreamSize(stream);
        closeByteStream(stream);
        assert(bytesRead + 4 == streamSize);
        return result;
    }
}

// Reads an element count, then allocates and loads the complete target-layout payload.
public RWStructuredBuffer<T> loadBuffer<T>(String path)
{
    return RWStructuredBuffer<T>::loadBuffer(path);
}

// Allocates a read-only resource and fills it from a count-prefixed structured-buffer file.
public extension<T> StructuredBuffer<T>
{
    public static StructuredBuffer<T> loadBuffer(String path)
    {
        ByteStream stream = openByteStream(path, ByteStreamMode.read);
        uint count = stream.read<uint>();
        StructuredBuffer<T> result = StructuredBuffer<T>::alloc(count);
        uint bytesRead = readByteStreamBuffer(stream, result);
        uint streamSize = getByteStreamSize(stream);
        closeByteStream(stream);
        assert(bytesRead + 4 == streamSize);
        return result;
    }
}

// Stores the element count followed by the complete raw contents of a structured buffer.
public uint storeBuffer<T>(String path, RWStructuredBuffer<T> buffer)
{
    uint count;
    uint stride;
    buffer.GetDimensions(count, stride);
    ByteStream stream = openByteStream(path, ByteStreamMode.write);
    uint bytesWritten = writeByteStream(stream, count);
    bytesWritten += writeByteStreamBuffer(stream, buffer);
    closeByteStream(stream);
    return bytesWritten;
}

// Stores the element count followed by the complete raw contents of a read-only buffer.
public uint storeBuffer<T>(String path, StructuredBuffer<T> buffer)
{
    uint count;
    uint stride;
    buffer.GetDimensions(count, stride);
    ByteStream stream = openByteStream(path, ByteStreamMode.write);
    uint bytesWritten = writeByteStream(stream, count);
    bytesWritten += writeByteStreamBuffer(stream, buffer);
    closeByteStream(stream);
    return bytesWritten;
}
