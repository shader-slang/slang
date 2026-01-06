implementing neural;

#ifndef UNIT_TEST
#define VISIBILITY_LEVEL public
#else
#define VISIBILITY_LEVEL internal
#endif


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

VISIBILITY_LEVEL enum AccessOp : uint32_t
{
    READ,
    WRITE,
    ACCUMULATE,
    ATOMIC_ADD,
}

#define COMMON_TYPE_CONSTRAINTS \
    where T : __BuiltinFloatingPointType \
    where U : __BuiltinFloatingPointType \
    where BufferType : IArrayAccessor<U>

[ForceInline]
internal static void readOneElement<T, U, BufferType, int NBytes, int BitsShiftPerRead>(BufferType buffer, int bufferIdx, int elementIdx, inout uint result)
    COMMON_TYPE_CONSTRAINTS
{
    const uint shift = BitsShiftPerRead * elementIdx;
    T convertedValue;
    convertedValue = __realCast<T>(buffer[bufferIdx]);
    switch (NBytes)
    {
    case 1:
        result |= uint(bit_cast<uint8_t>(convertedValue)) << shift;
        break;
    case 2:
        result |= uint(bit_cast<uint16_t>(convertedValue)) << shift;
        break;
    case 4:
        result |= uint(bit_cast<uint>(convertedValue)) << shift;
        break;
    default:
        static_assert(false, "Unsupported data type T");
    }
}

[ForceInline]
internal static void writeOneElement<T, U, BufferType, int NBytes, int BitsShiftPerWrite, AccessOp Op>(inout BufferType buffer, int bufferIdx, int elementIdx, uint value)
    COMMON_TYPE_CONSTRAINTS
{
    const uint shift = BitsShiftPerWrite * elementIdx;
    U convertedValue;
    switch (NBytes)
    {
    case 1:
        convertedValue = __realCast<U>(bit_cast<T>((uint8_t)(value >> shift)));
        break;
    case 2:
        convertedValue = __realCast<U>(bit_cast<T>((uint16_t)(value >> shift)));
        break;
    case 4:
        convertedValue = __realCast<U>(bit_cast<T>((uint)(value >> shift)));
        break;
    default:
        static_assert(false, "Unsupported data type T");
    }

    switch (Op)
    {
    case AccessOp.WRITE:
        buffer[bufferIdx] = convertedValue;
        break;
    case AccessOp.ACCUMULATE:
        buffer[bufferIdx] = buffer[bufferIdx] +convertedValue;
        break;
    case AccessOp.ATOMIC_ADD:
        buffer.atomicAdd(bufferIdx, convertedValue);
        break;
    default:
        static_assert(false, "Unsupported access operation");
    }
}

[ForceInline]
internal static void accessUint4Aligned<AccessOp Op, T, U, BufferType>(inout BufferType buffer, int startIndex, inout uint4 value)
    COMMON_TYPE_CONSTRAINTS
{
    const int nBytes = sizeof(T);
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
                readOneElement<T, U, BufferType, nBytes, BitsShiftPerWrite>(buffer, index, j, value[i]);
                break;
            case AccessOp.WRITE:
            case AccessOp.ACCUMULATE:
            case AccessOp.ATOMIC_ADD:
                writeOneElement<T, U, BufferType, nBytes, BitsShiftPerWrite, Op>(buffer, index, j, value[i]);
                break;
            default:
                static_assert(false, "Unsupported access operation");
            }
        }
    }
}

internal void accessUint4<AccessOp Op, T, U, BufferType, bool IsAligned, int Stride>(BufferType buffer, int baseIndex, int startIndex, inout uint4 value)
    COMMON_TYPE_CONSTRAINTS
{
    if (IsAligned)
    {
        // Call the aligned version of readUint4 which is branchless.
        accessUint4Aligned<Op, T, U, BufferType>(buffer, startIndex, value);
        return;
    }

    if (Op == AccessOp.READ)
        value = uint4(0, 0, 0, 0);

    // T is the type of source (read) or destination (write) data type. We will always pack few elements into a uint4.
    // So T will determine how many elements we can pack into a uint4.
    // If U is different from T, we will first convert from U to T (in read operation) or from T to U (in write operation).
    // But U will not determined how many elements we can read or write, only T will.
    const int nBytes = sizeof(T);
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
                readOneElement<T, U, BufferType, nBytes, BitsShiftPerRead>(buffer, index, j, value[i]);
                break;
            case AccessOp.WRITE:
            case AccessOp.ACCUMULATE:
            case AccessOp.ATOMIC_ADD:
                writeOneElement<T, U, BufferType, nBytes, BitsShiftPerRead, Op>(buffer, index, j, value[i]);
                break;
            default:
                static_assert(false, "Unsupported access operation");
            }
        }
    }
}
