// SPIR-V and GLSL have no floating-point atomic compare-exchange, so a compare-exchange on a
// `float` is done as a bitwise one on a `uint` view of the same buffer. See
// atomic-double-compare-exchange.slang for `double`.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -vk -shaderobj -output-using-type -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -cuda -shaderobj -output-using-type

struct Item
{
    uint tag;
    Atomic<float> value;
    Atomic<float> values[3];
}

//TEST_INPUT:ubuffer(data=[0.0 2.0], stride=4):name=floats
RWStructuredBuffer<Atomic<float>> floats;

//TEST_INPUT:ubuffer(data=[7 0 0 0 0 8 0 0 0 0], stride=20):name=items
RWStructuredBuffer<Item> items;

//TEST_INPUT:ubuffer(data=[2.0], stride=4):name=plainFloats
RWStructuredBuffer<float> plainFloats;

//TEST_INPUT:ubuffer(data=[9 0 9], stride=4):name=bytes
RWByteAddressBuffer bytes;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // The stored value matches, so it is replaced.
    outputBuffer[0] = asuint(floats[0].compareExchange(0.0f, 1.0f));
    // The stored value does not match, so it is kept.
    outputBuffer[1] = asuint(floats[1].compareExchange(0.0f, 5.0f));
    // The comparison is bitwise: -0.0 does not match a stored 0.0.
    outputBuffer[2] = asuint(items[1].value.compareExchange(-0.0f, 3.0f));
    // A dynamic buffer index and an array element inside the struct.
    uint i = tid.x + 1;
    outputBuffer[3] = asuint(items[i].values[2].compareExchange(0.0f, 4.0f));

    outputBuffer[4] = asuint(floats[0].load());
    outputBuffer[5] = asuint(floats[1].load());
    outputBuffer[6] = asuint(items[1].value.load());
    outputBuffer[7] = asuint(items[1].values[2].load());
    // The neighbours of the exchanged values are untouched.
    outputBuffer[8] = items[1].tag;
    outputBuffer[9] = asuint(items[1].values[1].load());
    outputBuffer[10] = asuint(items[0].values[2].load());
    outputBuffer[11] = items[0].tag;

    // The HLSL-style intrinsics take the same path.
    float original;
    bytes.InterlockedCompareExchangeFloatBitwise(4, 0.0f, 6.0f, original);
    outputBuffer[12] = asuint(original);
    outputBuffer[13] = bytes.Load(4);
    outputBuffer[14] = bytes.Load(8);
    InterlockedCompareExchange(plainFloats[0], 2.0f, 7.0f, original);
    outputBuffer[15] = asuint(plainFloats[0]);
}

// SPIRV-NOT: OpAtomicCompareExchange %float
// SPIRV: OpAtomicCompareExchange %uint
// SPIRV-NOT: OpAtomicCompareExchange %float

// GLSL: atomicCompSwap({{.*}}floatBitsToUint
// GLSL-NOT: atomicCompSwap({{[^;]*}}, 0.0

// BUF: 0
// BUF-NEXT: 1073741824
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 1065353216
// BUF-NEXT: 1073741824
// BUF-NEXT: 0
// BUF-NEXT: 1082130432
// BUF-NEXT: 8
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 7
// BUF-NEXT: 0
// BUF-NEXT: 1086324736
// BUF-NEXT: 9
// BUF-NEXT: 1088421888
