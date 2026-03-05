// Dynamic dispatch through StructuredBuffer<IFoo> elements where the buffer
// is pre-populated with existential values of different concrete types.
// This is a core pattern for material/light systems: a buffer of heterogeneous
// objects accessed via a common interface.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

[anyValueSize(8)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2.0; }
}

//TEST_INPUT: type_conformance ImplA:IFoo = 1
//TEST_INPUT: type_conformance ImplB:IFoo = 2

// Pre-populated buffer with two existential elements.
// Each element layout: RTTI(uint2) + WitnessTableID(uint2) + AnyValue(8 bytes)
//   = 24 bytes = 6 uint32s per element
// Element 0: ImplA(v=10.0), WitnessTableID=1
// Element 1: ImplB(v=3.0),  WitnessTableID=2
//TEST_INPUT: set inputBuffer = ubuffer(data=[0 0 1 0 10.0f 0  0 0 2 0 3.0f 0], stride=4)
StructuredBuffer<IFoo> inputBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // ImplA.getValue() returns v directly = 10.0
    outputBuffer[0] = inputBuffer[0].getValue();

    // ImplB.getValue() returns v * 2.0 = 3.0 * 2.0 = 6.0
    outputBuffer[1] = inputBuffer[1].getValue();

    // Read the same element twice to verify consistency
    outputBuffer[2] = inputBuffer[0].getValue();
    outputBuffer[3] = inputBuffer[1].getValue();

    // CHECK: 10.0
    // CHECK: 6.0
    // CHECK: 10.0
    // CHECK: 6.0
}
