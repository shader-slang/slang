implementing neural;

interface IArrayAccessor<T> : IRWArray<T>
{
    internal void atomicAdd(int index, T value)
    {
        static_assert(false, "atomicAdd is not supported for IArrayAccessor");
    }
}

internal extension<T> RWStructuredBuffer<T> : IArrayAccessor<T>
{
    override internal void atomicAdd(int index, T value)
    {
        __atomic_add(this[index], value);
    }
}

internal extension<T, int N> Array<T, N> : IArrayAccessor<T> {}

internal enum AccessOp
{
    READ,
    WRITE,
    ATOMIC_ADD,
}

[ForceInline]
internal static void readOneElement<T, BufferType, int NBytes, int BitsShiftPerRead>(BufferType buffer, int bufferIdx, int elementIdx, inout uint result)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where BufferType : IArrayAccessor<T>
{
    const uint shift = BitsShiftPerRead * elementIdx;
    switch (NBytes)
    {
    case 1:
        result |= uint(bit_cast<uint8_t>(buffer[bufferIdx])) << shift;
        break;
    case 2:
        result |= uint(bit_cast<uint16_t>(buffer[bufferIdx])) << shift;
        break;
    case 4:
        result |= uint(bit_cast<uint>(buffer[bufferIdx])) << shift;
        break;
    default:
        static_assert(false, "Unsupported data type T");
    }
}

[ForceInline]
internal static void writeOneElement<T, BufferType, int NBytes, int BitsShiftPerWrite>(out BufferType buffer, int bufferIdx, int elementIdx, uint value)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where BufferType : IArrayAccessor<T>
{
    const uint shift = BitsShiftPerWrite * elementIdx;
    switch (NBytes)
    {
    case 1:
        buffer[bufferIdx] = bit_cast<T>((uint8_t)(value >> shift));
        break;
    case 2:
        buffer[bufferIdx] = bit_cast<T>((uint16_t)(value >> shift));
        break;
    case 4:
        buffer[bufferIdx] = bit_cast<T>((uint)(value >> shift));
        break;
    default:
        static_assert(false, "Unsupported data type T");
    }
}

[ForceInline]
internal static void atomicAddOneElement<T, BufferType, int NBytes, int BitsShiftPerWrite>(inout BufferType buffer, int bufferIdx, int elementIdx, uint value)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where BufferType : IArrayAccessor<T>
{
    const uint shift = BitsShiftPerWrite * elementIdx;
    T convertedValue;
    switch (NBytes)
    {
    case 1:
        convertedValue = T(bit_cast<uint8_t>((uint8_t)(value >> shift)));
        break;
    case 2:
        convertedValue = T(bit_cast<half>((uint16_t)(value >> shift)));
        break;
    case 4:
        convertedValue = T(bit_cast<float>((uint)(value >> shift)));
        break;
    default:
        static_assert(false, "Unsupported data type T");
    }

    buffer.atomicAdd(bufferIdx, convertedValue);
}

[ForceInline]
internal static void accessUint4Aligned<AccessOp Op, T, BufferType>( out BufferType buffer, int startIndex, inout uint4 value)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where BufferType : IArrayAccessor<T>
{
    // TODO: for now, we just fixed the type to be half for all the internal operations.
    // We can extend this to support other types in the future.
    const int nBytes = sizeof(half);
    const int WritePerElement = 4 / nBytes;
    const int BitsShiftPerWrite = 32 / WritePerElement;

    if (Op == AccessOp.READ)
        value = uint4(0, 0, 0, 0);

    [ForceUnroll]
    for (int i = 0; i < 4; i++)
    {
        [ForceUnroll]
        for (int j = 0; j < WritePerElement; j++)
        {
            int index = startIndex + i * WritePerElement + j;
            switch (Op)
            {
            case AccessOp.READ:
                readOneElement<T, BufferType, nBytes, BitsShiftPerWrite>(buffer, index, j, value[i]);
                break;
            case AccessOp.WRITE:
                writeOneElement<T, BufferType, nBytes, BitsShiftPerWrite>(buffer, index, j, value[i]);
                break;
            case AccessOp.ATOMIC_ADD:
                atomicAddOneElement<T, BufferType, nBytes, BitsShiftPerWrite>(buffer, index, j, value[i]);
                break;
            default:
                static_assert(false, "Unsupported access operation");
            }
        }
    }
}

internal void accessUint4<AccessOp Op, T, BufferType, bool IsAligned, int Stride>(BufferType buffer, int baseIndex, int startIndex, inout uint4 value)
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where BufferType : IArrayAccessor<T>
{
    if (IsAligned)
    {
        // Call the aligned version of readUint4 which is branchless.
        accessUint4Aligned<Op, T, BufferType>(buffer, startIndex, value);
        return;
    }

    if (Op == AccessOp.READ)
        value = uint4(0, 0, 0, 0);

    // TODO: for now, we just fixed the type to be half for all the internal operations.
    // We can extend this to support other types in the future.
    const int nBytes = sizeof(half);
    const int ReadPerElement = 4 / nBytes;
    const int BitsShiftPerRead = 32 / ReadPerElement;

    const int x = (startIndex - baseIndex) % Stride;

    // end address of this read [address+length-1]
    const int endAddress = (x + 4 * ReadPerElement - 1);

    // this is same as paddingCount = endAddress < AlignedStride ? 0 : AlignedStride - endAddress + 1
    const int paddingCount = max<int>(0, endAddress - Stride + 1);
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
                return;
            }

            int index = (startIndex + offset);
            switch (Op)
            {
            case AccessOp.READ:
                readOneElement<T, BufferType, nBytes, BitsShiftPerRead>(buffer, index, j, value[i]);
                break;
            case AccessOp.WRITE:
                writeOneElement<T, BufferType, nBytes, BitsShiftPerRead>(buffer, index, j, value[i]);
                break;
            case AccessOp.ATOMIC_ADD:
                atomicAddOneElement<T, BufferType, nBytes, BitsShiftPerRead>(buffer, index, j, value[i]);
                break;
            default:
                static_assert(false, "Unsupported access operation");
            }
        }
    }
}
