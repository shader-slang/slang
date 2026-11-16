// Thread-group-shared memory passed to an `out` or `inout` parameter, or as a `[mutating]`
// receiver, is written when the call returns. A common pattern has one invocation receive the old
// value of an atomic in a `groupshared` variable, and a barrier then publish it to the group.
// Each case below writes shared memory that way from the first invocation, including an `int`
// element passed to an `inout uint` parameter through an implicit conversion, and every
// invocation reads the results after the barrier.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type

//TEST_INPUT:ubuffer(data=[5], stride=4):name=counters
RWStructuredBuffer<uint> counters;

//TEST_INPUT:ubuffer(data=[0 7], stride=4):name=counterBytes
RWByteAddressBuffer counterBytes;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Counter
{
    uint n;

    [mutating]
    void bump()
    {
        n += 10;
    }
}

groupshared uint gOffset;
groupshared uint gOffsets[2];
groupshared int gValue;
groupshared int gConverted[2];
groupshared Counter gCounter;

void writeOut(out int v)
{
    v = 3;
}

void addTwo(inout uint v)
{
    v += 2;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    if (i == 0)
    {
        InterlockedAdd(counters[0], 4u, gOffset);
        counterBytes.InterlockedAdd(4, 4u, gOffsets[1]);
        writeOut(gValue);
        gConverted[1] = 40;
        addTwo(gConverted[1]);
        gCounter.n = 1;
        gCounter.bump();
    }
    GroupMemoryBarrierWithGroupSync();

    outputBuffer[i] = gOffset + i;
    outputBuffer[4 + i] = gOffsets[1] + i;
    outputBuffer[8 + i] = uint(gValue) + i;
    outputBuffer[12 + i] = uint(gConverted[1]) + i;
    outputBuffer[16 + i] = gCounter.n + i;
}

// CHECK: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 9
// CHECK-NEXT: 10
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 42
// CHECK-NEXT: 43
// CHECK-NEXT: 44
// CHECK-NEXT: 45
// CHECK-NEXT: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 13
// CHECK-NEXT: 14
