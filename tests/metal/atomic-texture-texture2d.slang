//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
//TEST(compute, metal):COMPARE_COMPUTE_EX:-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -output-using-type

//METALLIB: @computeMain

//TEST_INPUT: RWTexture2D(format=R32Uint, size=15, content=one):name uintTexture2D
RWTexture2D<uint> uintTexture2D;
//TEST_INPUT: RWTexture2D(format=R32Sint, size=15, content=one):name intTexture2D
RWTexture2D<int> intTexture2D;

//TEST_INPUT: RWTexture2D(format=R32Uint, size=15, content=one, arrayLength=2):name uintTexture2DArray
RWTexture2DArray<uint> uintTexture2DArray;
//TEST_INPUT: RWTexture2D(format=R32Sint, size=15, content=one, arrayLength=2):name intTexture2DArray
RWTexture2DArray<int> intTexture2DArray;

//TEST_INPUT: ubuffer(data=[0], stride=1, count=368):out,name outBuffer
RWStructuredBuffer<int> outBuffer;

void test()
{
    int valInt = 3;
    int originalValueInt = 999;
    int compareValueInt = 1;

    uint valUInt = 3;
    uint originalValueUInt = 999;
    int compareValueUInt = 1;

    // Texture2D
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
// METAL: .atomic_compare_exchange_weak

// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
// METAL: .atomic_compare_exchange_weak
    int o = 0;
    int i = 0;
    InterlockedAdd(uintTexture2D[uint2(i++, 0)], valUInt);
    InterlockedAnd(uintTexture2D[uint2(i++, 0)], valUInt);
    InterlockedMax(uintTexture2D[uint2(i++, 0)], valUInt);
    InterlockedMin(uintTexture2D[uint2(i++, 0)], valUInt);
    InterlockedOr(uintTexture2D[uint2(i++, 0)], valUInt);
    InterlockedXor(uintTexture2D[uint2(i++, 0)], valUInt);
    InterlockedAdd(uintTexture2D[uint2(i++, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedAnd(uintTexture2D[uint2(i++, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedMax(uintTexture2D[uint2(i++, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedMin(uintTexture2D[uint2(i++, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedOr(uintTexture2D[uint2(i++, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedXor(uintTexture2D[uint2(i++, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedExchange(uintTexture2D[uint2(i++, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedCompareExchange(uintTexture2D[uint2(i++, 0)], valUInt, compareValueUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedCompareStore(uintTexture2D[uint2(i++, 0)], valUInt, compareValueUInt);

    i = 0;
    InterlockedAdd(intTexture2D[uint2(i++, 0)], valInt);
    InterlockedAnd(intTexture2D[uint2(i++, 0)], valInt);
    InterlockedMax(intTexture2D[uint2(i++, 0)], valInt);
    InterlockedMin(intTexture2D[uint2(i++, 0)], valInt);
    InterlockedOr(intTexture2D[uint2(i++, 0)], valInt);
    InterlockedXor(intTexture2D[uint2(i++, 0)], valInt);
    InterlockedAdd(intTexture2D[uint2(i++, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedAnd(intTexture2D[uint2(i++, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedMax(intTexture2D[uint2(i++, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedMin(intTexture2D[uint2(i++, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedOr(intTexture2D[uint2(i++, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedXor(intTexture2D[uint2(i++, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedExchange(intTexture2D[uint2(i++, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedCompareExchange(intTexture2D[uint2(i++, 0)], valInt, compareValueInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedCompareStore(intTexture2D[uint2(i++, 0)], valInt, compareValueUInt);

    // Texture2DArray
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
// METAL: .atomic_compare_exchange_weak

// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
// METAL: .atomic_compare_exchange_weak
    i = 0;
    InterlockedAdd(uintTexture2DArray[uint3(i++, 0, 0)], valUInt);
    InterlockedAnd(uintTexture2DArray[uint3(i++, 0, 0)], valUInt);
    InterlockedMax(uintTexture2DArray[uint3(i++, 0, 0)], valUInt);
    InterlockedMin(uintTexture2DArray[uint3(i++, 0, 0)], valUInt);
    InterlockedOr(uintTexture2DArray[uint3(i++, 0, 0)], valUInt);
    InterlockedXor(uintTexture2DArray[uint3(i++, 0, 0)], valUInt);
    InterlockedAdd(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedAnd(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedMax(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedMin(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedOr(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedXor(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedExchange(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedCompareExchange(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, compareValueUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedCompareStore(uintTexture2DArray[uint3(i++, 0, 0)], valUInt, compareValueUInt);

    i = 0;
    InterlockedAdd(intTexture2DArray[uint3(i++, 0, 0)], valInt);
    InterlockedAnd(intTexture2DArray[uint3(i++, 0, 0)], valInt);
    InterlockedMax(intTexture2DArray[uint3(i++, 0, 0)], valInt);
    InterlockedMin(intTexture2DArray[uint3(i++, 0, 0)], valInt);
    InterlockedOr(intTexture2DArray[uint3(i++, 0, 0)], valInt);
    InterlockedXor(intTexture2DArray[uint3(i++, 0, 0)], valInt);
    InterlockedAdd(intTexture2DArray[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedAnd(intTexture2DArray[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedMax(intTexture2DArray[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedMin(intTexture2DArray[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedOr(intTexture2DArray[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedXor(intTexture2DArray[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedExchange(intTexture2DArray[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedCompareExchange(intTexture2DArray[uint3(i++, 0, 0)], valInt, compareValueInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedCompareStore(intTexture2DArray[uint3(i++, 0, 0)], valUInt, compareValueUInt);

    // Copy to output
    int test_count = i;
    for (i=0; i<test_count; i++) {
        outBuffer[o + test_count*0 + i] = uintTexture2D[uint2(i, 0)];
        outBuffer[o + test_count*1 + i] = intTexture2D[uint2(i, 0)];
        outBuffer[o + test_count*2 + i] = uintTexture2DArray[uint3(i, 0, 0)];
        outBuffer[o + test_count*3 + i] = intTexture2DArray[uint3(i, 0, 0)];
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    test();
}
