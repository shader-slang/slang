// Test that ineligible types (structs, arrays) do NOT use OpBitcast per SPIR-V spec
// Per spec: OpBitcast only allows "scalar or vector of numerical type" or pointers
// Structs and arrays are NOT eligible, so they MUST be lowered to component operations

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// CHECK-DAG: OpBitcast %int %
// CHECK-DAG: OpBitcast %float %

// CHECK-DAG: OpShiftLeftLogical
// CHECK-DAG: OpBitwiseOr

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

struct IntPair
{
    int x;
    int y;
};

struct FloatPair
{
    float a;
    float b;
};

//TEST_INPUT:ubuffer(data=[42 84 305419896 2596069104], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=floatBuffer
RWStructuredBuffer<float> floatBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=intBuffer
RWStructuredBuffer<int> intBuffer;

// OUT: 42
// OUT: 84
// OUT: 305419896

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding

    // Struct types - ineligible for optimization
    IntPair ip = { asint(inputBuffer[0]), asint(inputBuffer[1]) };
    intBuffer[0] = ip.x;  // Force preservation of int
    intBuffer[1] = ip.y;
    FloatPair fp = bit_cast<FloatPair>(ip);
    floatBuffer[0] = fp.a;  // Force preservation of float bitcast
    floatBuffer[1] = fp.b;

    // Array types - ineligible for optimization
    uint uintArray[2] = { inputBuffer[2], inputBuffer[3] };
    uint64_t u64 = bit_cast<uint64_t>(uintArray);

    // Store results to prevent optimization
    outputBuffer[0] = asuint(floatBuffer[0]);
    outputBuffer[1] = asuint(floatBuffer[1]);
    outputBuffer[2] = uint(u64 & 0xFFFFFFFF);
}
