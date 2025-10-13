// bitcast-ineligible-types.slang
// Test that ineligible types (structs, arrays) do NOT use the optimization and are lowered

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Ineligible types should be lowered (scalarized and bitcast component-wise)
// Structs are completely scalarized - no single OpBitcast on the whole struct type
// CHECK-DAG: OpBitcast %int %
// CHECK-DAG: OpBitcast %float %
// Arrays are combined using shifts and bitwise operations
// CHECK-DAG: OpShiftLeftLogical
// CHECK-DAG: OpBitwiseOr

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
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding
    
    // Struct types - ineligible for optimization
    IntPair ip = { asint(inputBuffer[0]), asint(inputBuffer[1]) };
    FloatPair fp = bit_cast<FloatPair>(ip);
    
    // Array types - ineligible for optimization  
    uint uintArray[2] = { inputBuffer[2], inputBuffer[3] };
    uint64_t u64 = bit_cast<uint64_t>(uintArray);
    
    // Store results to prevent optimization
    outputBuffer[0] = asuint(fp.a);
    outputBuffer[1] = asuint(fp.b);
    outputBuffer[2] = uint(u64 & 0xFFFFFFFF);
}
