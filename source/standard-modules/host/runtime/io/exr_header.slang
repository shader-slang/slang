// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.io.byte_stream;
import slang.runtime.core.simple_assert;

/// Reads a complete scalar from a file header, rejecting truncated input.
T readEXRField<T>(ByteStream stream)
{
    T value;
    if (readByteStream(stream, value) != sizeof(T))
        fail("Truncated EXR header");
    return value;
}

uint readEXRByte(ByteStream stream)
{
    uint value = 0;
    if (readByteStreamBytes(stream, value, 0, 1) != 1)
        fail("Truncated EXR header");
    return value;
}

/// Consumes a null-terminated attribute name or type and compares it with a known spelling.
bool matchEXRString(ByteStream stream, String expected, out bool empty)
{
    uint length = getStringByteCount(expected);
    bool matches = true;
    uint index = 0;
    for (;;)
    {
        uint value = readEXRByte(stream);
        if (value == 0)
        {
            empty = index == 0;
            return matches && index == length;
        }
        if (index >= length)
            matches = false;
        else if (value != readStringByte(expected, index))
            matches = false;
        index++;
    }
}

/// Reads the pixel dimensions without decoding the image. The native codec validates the full
/// format when loading; allocation uses the data window, including files with a nonzero origin.
public uint2 readEXRDimensions(String path)
{
    ByteStream stream = openByteStream(path, ByteStreamMode.read);
    if (readEXRField<uint>(stream) != 20000630)
        fail("Expected an OpenEXR image");
    uint version = readEXRField<uint>(stream);
    if (version % 256 != 2 || (version / 512) % 2 != 0 || version / 2048 != 0)
        fail("Expected a single-part scanline EXR");
    for (;;)
    {
        bool empty;
        bool isDataWindow = matchEXRString(stream, "dataWindow", empty);
        if (empty)
            fail("EXR header has no data window");
        bool isBox = matchEXRString(stream, "box2i", empty);
        uint byteCount = readEXRField<uint>(stream);
        if (isDataWindow && isBox && byteCount == 16)
        {
            int4 bounds = readEXRField<int4>(stream);
            closeByteStream(stream);
            uint widthMinusOne = uint(bounds.z) - uint(bounds.x);
            uint heightMinusOne = uint(bounds.w) - uint(bounds.y);
            if (bounds.z < bounds.x || bounds.w < bounds.y || widthMinusOne >= 0x7fffffff ||
                heightMinusOne >= 0x7fffffff)
                fail("Invalid EXR data window");
            return uint2(widthMinusOne + 1, heightMinusOne + 1);
        }
        // Header attributes are small. Reading scalar bytes avoids allocating based on an
        // untrusted attribute size and fails at EOF if the attribute is truncated.
        for (uint i = 0; i < byteCount; ++i)
            readEXRByte(stream);
    }
}
