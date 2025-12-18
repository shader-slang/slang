//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
//TEST(compute, metal):COMPARE_COMPUTE_EX:-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -output-using-type

//METALLIB: @computeMain

//TEST_INPUT: RWTexture3D(format=R32Uint, size=15, content=one):name uintTexture3D
RWTexture3D<uint> uintTexture3D;
//TEST_INPUT: RWTexture3D(format=R32Sint, size=15, content=one):name intTexture3D
RWTexture3D<int> intTexture3D;

//TEST_INPUT: ubuffer(data=[0], stride=1, count=184):out,name outBuffer
RWStructuredBuffer<int> outBuffer;

void test()
{
    int valInt = 3;
    int originalValueInt = 999;
    int compareValueInt = 1;

    uint valUInt = 3;
    uint originalValueUInt = 999;
    int compareValueUInt = 1;

    // Texture3D
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
    InterlockedAdd(uintTexture3D[uint3(i++, 0, 0)], valUInt);
    InterlockedAnd(uintTexture3D[uint3(i++, 0, 0)], valUInt);
    InterlockedMax(uintTexture3D[uint3(i++, 0, 0)], valUInt);
    InterlockedMin(uintTexture3D[uint3(i++, 0, 0)], valUInt);
    InterlockedOr(uintTexture3D[uint3(i++, 0, 0)], valUInt);
    InterlockedXor(uintTexture3D[uint3(i++, 0, 0)], valUInt);
    InterlockedAdd(uintTexture3D[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedAnd(uintTexture3D[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedMax(uintTexture3D[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedMin(uintTexture3D[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedOr(uintTexture3D[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedXor(uintTexture3D[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedExchange(uintTexture3D[uint3(i++, 0, 0)], valUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedCompareExchange(uintTexture3D[uint3(i++, 0, 0)], valUInt, compareValueUInt, originalValueUInt);
    outBuffer[o++] = originalValueUInt;
    InterlockedCompareStore(uintTexture3D[uint3(i++, 0, 0)], valUInt, compareValueUInt);

    i = 0;
    InterlockedAdd(intTexture3D[uint3(i++, 0, 0)], valInt);
    InterlockedAnd(intTexture3D[uint3(i++, 0, 0)], valInt);
    InterlockedMax(intTexture3D[uint3(i++, 0, 0)], valInt);
    InterlockedMin(intTexture3D[uint3(i++, 0, 0)], valInt);
    InterlockedOr(intTexture3D[uint3(i++, 0, 0)], valInt);
    InterlockedXor(intTexture3D[uint3(i++, 0, 0)], valInt);
    InterlockedAdd(intTexture3D[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedAnd(intTexture3D[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedMax(intTexture3D[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedMin(intTexture3D[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedOr(intTexture3D[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedXor(intTexture3D[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedExchange(intTexture3D[uint3(i++, 0, 0)], valInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedCompareExchange(intTexture3D[uint3(i++, 0, 0)], valInt, compareValueInt, originalValueInt);
    outBuffer[o++] = originalValueInt;
    InterlockedCompareStore(intTexture3D[uint3(i++, 0, 0)], valInt, compareValueUInt);


    // Copy to output
    int test_count = i;
    for (i=0; i<test_count; i++) {
        outBuffer[o + test_count*0 + i] = uintTexture3D[uint3(i, 0, 0)];
        outBuffer[o + test_count*1 + i] = intTexture3D[uint3(i, 0, 0)];
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    test();
}
