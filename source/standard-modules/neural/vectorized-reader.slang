implementing neural;

// Extension for RWStructuredBuffer to implement IArray interface such that we can implement readUint4 once, and
// use it for both storage buffer and thread local array.
internal extension<T> RWStructuredBuffer<T> : IArray<T>
{
    internal __subscript(int index) -> T { get { return this[index]; }}
    internal int getCount()
    {
        int count = 0;
        int stride = 0;
        this.GetDimensions(count, stride);
        return count;
    }
}

[ForceInline]
internal static uint4 readUint4<T, BufferType>(BufferType buffer, int startIndex)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where BufferType : IArray<T>
{
    uint4 result = uint4(0, 0, 0, 0);
    const int nBytes = sizeof(T);
    const int ReadPerElement = 4 / nBytes;
    const int BitsShiftPerRead = 32 / ReadPerElement;
    [ForceUnroll]
    for (int i = 0; i < 4; i++)
    {
        [ForceUnroll]
        for (int j = 0; j < ReadPerElement; j++)
        {
            int index = startIndex + i * ReadPerElement + j;
            switch (nBytes)
            {
            case 1:
                result[i] |= uint(bit_cast<uint8_t>(buffer[index]))
                                << (BitsShiftPerRead * (ReadPerElement - 1 - j));
                break;
            case 2:
                result[i] |= uint(bit_cast<uint16_t>(buffer[index]))
                                << (BitsShiftPerRead * (ReadPerElement - 1 - j));
                break;
            case 4:
                result[i] |= uint(bit_cast<uint>(buffer[index]))
                                << (BitsShiftPerRead * (ReadPerElement - 1 - j));
                break;
            default:
                static_assert(false, "Unsupported data type T");
                return uint4(0);
            }
        }
    }
    return result;
}

internal uint4 readUint4<T, BufferType, bool IsAligned, uint Stride, uint AlignedStride>(BufferType buffer, int baseIndex, int startIndex)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where BufferType : IArray<T>
{
    if (IsAligned)
    {
        // Call the aligned version of readUint4 which is branchless.
        return readUint4<T, BufferType>(buffer, startIndex);
    }

    uint4 result = uint4(0, 0, 0, 0);
    const int nBytes = sizeof(T);
    const int ReadPerElement = 4 / nBytes;
    const int BitsShiftPerRead = 32 / ReadPerElement;

    const int x = (startIndex - baseIndex) % Stride;

    // end address of this read [address+length-1]
    const int endAddress = (x + 4 * ReadPerElement - 1);

    // this is same as paddingCount = endAddress < AlignedStride ? 0 : AlignedStride - endAddress + 1
    const int paddingCount = max(0, endAddress - Stride + 1);
    const int elementsToRead = (4 * ReadPerElement) - paddingCount;


    [ForceUnroll]
    for (int i = 0; i < 4; i++)
    {
        int offset = i * ReadPerElement;
        [ForceUnroll]
        for (int j = 0; j < ReadPerElement; j++)
        {
            // 4 * ReadPerElement is the total number of elements we can read from the buffer.
            // paddingCount is the number of the elements we need to pad.
            // e.g. if ReadPerElement is 2, paddingCount is 4.Because (4 * 2 - 4 == 4), so we can
            // just stop reading when offset bigger than 3.
            offset += j;
            if (offset >= elementsToRead)
            {
                return result;
            }

            int index = (startIndex + offset);
            switch (nBytes)
            {
            case 1:
                result[i] |= uint(bit_cast<uint8_t>(buffer[index]))
                                << (BitsShiftPerRead * (ReadPerElement - 1 - j));
                break;
            case 2:
                result[i] |= uint(bit_cast<uint16_t>(buffer[index]))
                                << (BitsShiftPerRead * (ReadPerElement - 1 - j));
                break;
            case 4:
                result[i] |= uint(bit_cast<uint>(buffer[index]))
                                << (BitsShiftPerRead * (ReadPerElement - 1 - j));
                break;
            default:
                static_assert(false, "Unsupported data type T");
                return uint4(0);
            }
        }
    }
    return result;
}
