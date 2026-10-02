//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -conformance Number<0>:ICode=0 -conformance Number<1>:ICode=1 -conformance Number<2>:ICode=2 -conformance Number<3>:ICode=3 -conformance Number<4>:ICode=4 -conformance Number<5>:ICode=5 -conformance Number<6>:ICode=6 -conformance Number<7>:ICode=7 -conformance Number<8>:ICode=8
// Diagnostic coverage below checks the same resource limit separately.
#pragma warning(disable:55219)

interface ICode
{
    static uint code();
}

struct Number<let N : uint> : ICode
{
    static uint code() { return N; }
}

uint readMarker(inout uint state)
{
    return state++;
}

// Nine conformers exceed the current duplication budget. The hint must leave a
// single shared helper and preserve its behavior rather than rejecting the program.
[__specializePerConformance]
void getCode<T : ICode>(T value, inout uint state)
{
    uint context = readMarker(state);
    state = T.code() + context;
}

//TEST_INPUT: type_conformance Number<0>:ICode = 0
//TEST_INPUT: type_conformance Number<1>:ICode = 1
//TEST_INPUT: type_conformance Number<2>:ICode = 2
//TEST_INPUT: type_conformance Number<3>:ICode = 3
//TEST_INPUT: type_conformance Number<4>:ICode = 4
//TEST_INPUT: type_conformance Number<5>:ICode = 5
//TEST_INPUT: type_conformance Number<6>:ICode = 6
//TEST_INPUT: type_conformance Number<7>:ICode = 7
//TEST_INPUT: type_conformance Number<8>:ICode = 8
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(9, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint state = 0;
    getCode(createDynamicObject<ICode>(tid.x, 0u), state);
    outputBuffer[tid.x] = state;
}

// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 5
// CHECK: 6
// CHECK: 7
// CHECK: 8
// HLSL: = readMarker
// HLSL-NOT: = readMarker
