// Regression test for https://github.com/shader-slang/slang/issues/11313
//
// Dynamic-dispatch handle types (WitnessTableID / RTTIHandle) are lowered to a
// fixed-layout 64-bit carrier in lowerHandleTypes() -- `uint2` on most targets,
// but `ulong` on Metal, because MSL cannot cast a vector to a pointer. This test
// deliberately does NOT inspect the carrier type. Instead it verifies the
// behavior the carrier must preserve on every target where dynamic dispatch is
// supported: two existentials of different concrete types each dispatch to their
// own implementation and never pick up the other's witness-table id.
//
// Both elements carry the same payload value (7); they differ only in concrete
// type. So the distinct outputs (7 vs 1007) come solely from dispatching to the
// correct implementation -- if the two witness ids were confused, the outputs
// would swap. The interleaved re-reads confirm each element's id is stable.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

[anyValueSize(8)]
interface IFoo
{
    int getValue();
}

struct ImplA : IFoo
{
    int v;
    int getValue() { return v; }
}

struct ImplB : IFoo
{
    int v;
    int getValue() { return v + 1000; }
}

//TEST_INPUT: type_conformance ImplA:IFoo = 1
//TEST_INPUT: type_conformance ImplB:IFoo = 2

// Two heterogeneous existential elements, both with payload v = 7.
// Layout per element: RTTI(8B) + WitnessTableID(8B) + AnyValue(8B) = 6 uint32s.
//   Element 0: ImplA, witness id = 1, v = 7
//   Element 1: ImplB, witness id = 2, v = 7
// The byte layout is identical whether the handle carrier is `uint2` or `ulong`.
//TEST_INPUT: set inputBuffer = ubuffer(data=[0 0 1 0 7 0  0 0 2 0 7 0], stride=4)
StructuredBuffer<IFoo> inputBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // ImplA.getValue() = v = 7
    outputBuffer[0] = inputBuffer[0].getValue();
    // ImplB.getValue() = v + 1000 = 1007
    outputBuffer[1] = inputBuffer[1].getValue();
    // Interleaved re-reads: each element keeps its own id, no swap.
    outputBuffer[2] = inputBuffer[1].getValue();
    outputBuffer[3] = inputBuffer[0].getValue();

    // CHECK: 7
    // CHECK: 1007
    // CHECK: 1007
    // CHECK: 7
}
