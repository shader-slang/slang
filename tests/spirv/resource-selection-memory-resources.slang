//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -output-using-type -emit-spirv-directly -Xslang -O0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -output-using-type -emit-spirv-directly -Xslang -O3
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -O0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -O3
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain

// Check that per-invocation selections between two different storage buffers, byte-address
// buffers and constant buffers read, write and update the chosen resource, and that the module
// needs no variable pointers. Each invocation picks the first resource when its index is even.

// SPIRV-NOT: VariablePointers
// SPIRV: OpEntryPoint
// GLSL: void main()

//TEST_INPUT: ubuffer(data=[10 11 12 13], stride=4):name inputA
StructuredBuffer<uint> inputA;
//TEST_INPUT: ubuffer(data=[20 21 22 23 24 25], stride=4):name inputB
StructuredBuffer<uint> inputB;
//TEST_INPUT: ubuffer(data=[0 0 0 0 0], stride=4):name scratchA
RWStructuredBuffer<uint> scratchA;
//TEST_INPUT: ubuffer(data=[0 0 0 0 0], stride=4):name scratchB
RWStructuredBuffer<uint> scratchB;
//TEST_INPUT: ubuffer(data=[0], stride=4):name countA
RWByteAddressBuffer countA;
//TEST_INPUT: ubuffer(data=[0], stride=4):name countB
RWByteAddressBuffer countB;

struct Constants
{
    uint value;
};
//TEST_INPUT: cbuffer(data=[7 0 0 0]):name constantsA
ConstantBuffer<Constants> constantsA;
//TEST_INPUT: cbuffer(data=[9 0 0 0]):name constantsB
ConstantBuffer<Constants> constantsB;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name results
RWStructuredBuffer<uint> results;

[numthreads(4, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    uint i = threadID.x;
    bool chooseA = (i & 1) == 0;

    StructuredBuffer<uint> input = chooseA ? inputA : inputB;
    RWStructuredBuffer<uint> scratch = chooseA ? scratchA : scratchB;
    RWByteAddressBuffer count = chooseA ? countA : countB;
    ConstantBuffer<Constants> constants = chooseA ? constantsA : constantsB;

    results[i] = input[i];
    uint elementCount, stride;
    input.GetDimensions(elementCount, stride);
    results[12 + i] = elementCount;
    results[16 + i] = constants.value;

    scratch[i] = 100 + i;
    InterlockedAdd(scratch[4], i + 1);
    uint previous;
    count.InterlockedAdd(0, i + 1, previous);

    AllMemoryBarrierWithGroupSync();

    // Scaling one buffer by 1000 shows which buffer each invocation wrote.
    results[4 + i] = scratchA[i] + 1000 * scratchB[i];
    if (i == 0)
    {
        results[8] = scratchA[4];
        results[9] = scratchB[4];
        results[10] = countA.Load(0);
        results[11] = countB.Load(0);
    }
}

// Reads through the selected StructuredBuffer.
// CHECK: 10
// CHECK-NEXT: 21
// CHECK-NEXT: 12
// CHECK-NEXT: 23
// Writes through the selected RWStructuredBuffer.
// CHECK-NEXT: 100
// CHECK-NEXT: 101000
// CHECK-NEXT: 102
// CHECK-NEXT: 103000
// Atomic adds through the selected RWStructuredBuffer and RWByteAddressBuffer.
// CHECK-NEXT: 4
// CHECK-NEXT: 6
// CHECK-NEXT: 4
// CHECK-NEXT: 6
// GetDimensions of the selected StructuredBuffer.
// CHECK-NEXT: 4
// CHECK-NEXT: 6
// CHECK-NEXT: 4
// CHECK-NEXT: 6
// Field reads through the selected ConstantBuffer.
// CHECK-NEXT: 7
// CHECK-NEXT: 9
// CHECK-NEXT: 7
// CHECK-NEXT: 9
