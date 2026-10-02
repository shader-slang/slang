//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -conformance First:IOperation=3 -conformance Second:IOperation=7
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry computeMain -conformance First:IOperation=3 -conformance Second:IOperation=7

// Context reads and acceptance belong to the whole concrete visit, not to a shared
// region before/after the interface call. Nonzero, nonconsecutive IDs exercise tag mapping.
import "specialize-per-conformance-first";
import "specialize-per-conformance-second";

uint readContext(inout uint state)
{
    return state++;
}

void acceptCandidate(uint candidate, inout uint state)
{
    state += candidate;
}

[__specializePerConformance]
[ForceInline]
void visit<G : IOperation>(G geometry, inout uint state)
{
    uint candidate;
    if (G.evaluate(readContext(state), candidate))
        acceptCandidate(candidate, state);
}

//TEST_INPUT: type_conformance First:IOperation = 3
//TEST_INPUT: type_conformance Second:IOperation = 7
//TEST_INPUT:ubuffer(data=[3 3 7 7], stride=4):name=ids
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
StructuredBuffer<uint> ids;
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint state = tid.x;
    visit(createDynamicObject<IOperation>(ids[tid.x], 0u), state);
    outputBuffer[tid.x] = state;
}

// CHECK: 11
// CHECK: 2
// CHECK: 3
// CHECK: 27

// Each concrete helper reads the current state before evaluating its operation,
// then accepts only on success. A single shared helper must fail this check.
// HLSL: = readContext
// HLSL: acceptCandidate
// HLSL: = readContext
// HLSL: acceptCandidate
// SPIRV: OpEntryPoint GLCompute
